/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     |
    \\  /    A nd           | For copyright notice see file Copyright
     \\/     M anipulation  |
-------------------------------------------------------------------------------
31-03-2022 Synthetik Applied Technologies: Added base plasticModel
-------------------------------------------------------------------------------
License
    This file is a derivative work of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "nonLinearPlasticModel.H"
#include "addToRunTimeSelectionTable.H"
#include "transformGeometricField.H"
#include "logVolFields.H"
#include "fvc.H"
#include "fvm.H"
#include "zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

// Construct from dictionary
Foam::nonLinearPlasticModel::nonLinearPlasticModel
(
    const word& name,
    const fvMesh& mesh,
    const dictionary& dict,
    const nonLinearGeometry::nonLinearType& nonLinGeom
)
:
    plasticModel(name, mesh, dict, nonLinGeom),
    updateBEbarConsistent_
    (
        dict.lookupOrDefault<Switch>("updateBEbarConsistent", true)
    )
{
    if (updateBEbarConsistent_)
    {
        Info<< "updateBEbarConsistent is active" << endl;
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::nonLinearPlasticModel::~nonLinearPlasticModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField> Foam::nonLinearPlasticModel::impK() const
{
    // Calculate scaling factor to ensure optimal convergence
    // This is similar to the tangent matrix in FE procedures

    // Calculate deviatoric trial stress
    const volSymmTensorField sTrial(mu_*dev(bEbarTrial()));

    const volScalarField Ibar(tr(bEbarTrial())/3.0);
    const volScalarField muBar(Ibar*mu_);

    // Magnitude of the deviatoric trial stress
    const volScalarField magSTrial
    (
        max(mag(sTrial), dimensionedScalar("SMALL", dimPressure, SMALL))
    );

    // Calculate scaling factor
    const volScalarField scaleFactor(1.0 - (2.0*muBar*DLambda()/magSTrial));

    return volScalarField::New
    (
        "impK",
        //mesh(),
        //(4.0/3.0)*mu_ + K_, // == 2*mu + lambda
        //zeroGradientFvPatchScalarField::typeName
        scaleFactor*(4.0/3.0)*mu_ + K_
    );
}


Foam::tmp<Foam::scalarField>
Foam::nonLinearPlasticModel::impK(const label patchi) const
{
    const symmTensorField& pbEbarTrial
    (
        bEbarTrial().boundaryField()[patchi]
    );
    // Calculate deviatoric trial stress
    const symmTensorField sTrial(mu_.value()*dev(pbEbarTrial));

    const scalarField Ibar(tr(pbEbarTrial)/3.0);
    const scalarField muBar(Ibar*mu_.value());

    // Magnitude of the deviatoric trial stress
    const scalarField magSTrial(max(mag(sTrial), small));

    // Calculate scaling factor
    const scalarField scaleFactor
    (
        1.0
      - (2.0*muBar*DLambda().boundaryField()[patchi]/magSTrial)
    );

    return scaleFactor*(4.0/3.0)*mu_.value() + K_.value();
}


void Foam::nonLinearPlasticModel::correct
(
    volSymmTensorField& sigma,
    const bool needUpdate
)
{
    // Update the deformation gradient field
    // Note: if true is returned, it means that linearised elasticity was
    // enforced by the solver via the enforceLinear switch
    if (updateF(sigma, mu_, K_))
    {
        return;
    }

    // Update the Jacobian of the total deformation gradient
    const volScalarField& J = this->J();
    volScalarField& DEpsilonPEq = this->DEpsilonPEqRef();
    volSymmTensorField& DEpsilonP = this->DEpsilonPRef();
    volSymmTensorField& bEbar = this->bEbarRef();
    volSymmTensorField& bEbarTrial = this->bEbarTrialRef();
    volSymmTensorField& plasticN = this->plasticNRef();
    volScalarField& DLambda = this->DLambdaRef();
    volScalarField& sigmaY = this->sigmaYRef();
    const volScalarField& sigmaYOld = sigmaY.oldTime();
    volScalarField& epsilonPEq = this->epsilonPEqRef();
    volSymmTensorField& epsilonP = this->epsilonPRef();
    const volScalarField& epsilonPEqOld = epsilonPEq.oldTime();

    // Calculate the relative deformation gradient with the volumetric term
    // removed
    const volTensorField relFbar(relF()/cbrt(relJ()));

    // Update bE trial
    bEbarTrial = transform(relFbar, bEbar.oldTime());

    // Calculate trial deviatoric stress
    const volSymmTensorField sTrial(mu_*dev(bEbarTrial));

    const volScalarField Ibar(tr(bEbarTrial)/3.0);
    const volScalarField muBar(Ibar*mu_);

    // Check for plastic loading
    // and calculate increment of plastic equivalent strain
    // i.e. the plastic multiplier

    // Store previous iteration for under-relaxation and calculation of plastic
    // residual in the solver
    DEpsilonP.storePrevIter();

    // Normalise residual in Newton method with respect to mag(bE)
    const scalar maxMagBE = max(max(mag(bEbarTrial)).value(), small);

    // Trial yield function
    // sigmaY is the Cauchy yield stress so we scale it by J
    const volScalarField fTrial(mag(sTrial) - sqrtTwoOverThree_*J*sigmaY);


    forAll(fTrial, cellI)
    {
        setCellValues(cellI);

        // Update plasticN, DLambda, DSigmaY and sigmaY for this cell
        this->updatePlasticity
        (
            plasticN[cellI],
            DLambda[cellI],
            sigmaY[cellI],
            sigmaYOld[cellI],
            fTrial[cellI],
            sTrial[cellI],
            epsilonPEqOld[cellI],
            muBar[cellI],
            maxMagBE,
            J[cellI]
        );
    }

    volSymmTensorField::Boundary& bplasticN = plasticN.boundaryFieldRef();
    volScalarField::Boundary& bDLambda = DLambda.boundaryFieldRef();
    volScalarField::Boundary& bsigmaY = sigmaY.boundaryFieldRef();
    forAll(fTrial.boundaryField(), patchI)
    {
        // Take references to the boundary patch fields for efficiency
        const scalarField& pfTrial = fTrial.boundaryField()[patchI];
        const symmTensorField& psTrial = sTrial.boundaryField()[patchI];
        symmTensorField& pN = bplasticN[patchI];
        scalarField& pDLambda = bDLambda[patchI];
        scalarField& psigmaY = bsigmaY[patchI];
        const scalarField& pmuBar = muBar.boundaryField()[patchI];

        const scalarField& psigmaYOld = sigmaYOld.boundaryField()[patchI];
        const scalarField& pepsilonPEqOld =
            epsilonPEqOld.boundaryField()[patchI];
        const scalarField& pJ = J.boundaryField()[patchI];

        forAll(pfTrial, faceI)
        {
            setVolPatchFaceValues(patchI, faceI);

            // Update plasticN, DLambda, DSigmaY and sigmaY for this face
            this->updatePlasticity
            (
                pN[faceI],
                pDLambda[faceI],
                psigmaY[faceI],
                psigmaYOld[faceI],
                pfTrial[faceI],
                psTrial[faceI],
                pepsilonPEqOld[faceI],
                pmuBar[faceI],
                maxMagBE,
                pJ[faceI]
            );
        }
    }

    // Update DEpsilonP and DEpsilonPEq
    DEpsilonPEq = sqrtTwoOverThree_*DLambda;
    DEpsilonP = Ibar*DLambda*plasticN;
    DEpsilonP.relax();

    epsilonPEq = epsilonPEq.oldTime() + DEpsilonPEq;
    epsilonP = epsilonP.oldTime() + DEpsilonP;

    // Calculate deviatoric stress
    const volSymmTensorField s(sTrial - 2*mu_*DEpsilonP);

    // Update bEbar
    const volSymmTensorField devBEbar(s/mu_);
    if (updateBEbarConsistent_)
    {
        bEbar = devBEbar + this->Ibar(devBEbar)*I;
    }
    else
    {
        bEbar = devBEbar + Ibar*I;
    }

    // Update hydrostatic stress (negative of pressure)
    updateSigmaHyd
    (
        0.5*K_*(sqr(J) - 1.0),
        (4.0/3.0)*mu_ + K_
    );

    // Update the Cauchy stress
    sigma = (sigmaHyd()*I + s)/J;
    sigma.correctBoundaryConditions();
}


void Foam::nonLinearPlasticModel::correct
(
    surfaceSymmTensorField& sigma,
    const bool needUpdate
)
{
    // Update the deformation gradient field
    // Note: if true is returned, it means that linearised elasticity was
    // enforced by the solver via the enforceLinear switch
    if (updateFf(sigma, mu_, K_))
    {
        return;
    }

    // Update the Jacobian of the total deformation gradient
    const surfaceScalarField& Jf = this->Jf();
    surfaceSymmTensorField& bEbarf = this->bEbarfRef();
    surfaceSymmTensorField& bEbarTrialf = this->bEbarTrialfRef();
    surfaceSymmTensorField& plasticNf = this->plasticNfRef();
    surfaceScalarField& DLambdaf = this->DLambdafRef();
    surfaceScalarField& sigmaYf = this->sigmaYfRef();
    const surfaceScalarField& sigmaYfOld = sigmaYf.oldTime();
    const surfaceScalarField& epsilonPEqfOld = epsilonPEqf().oldTime();

    // Calculate the relative Jacobian
    const surfaceScalarField relJ(Jf/Jf.oldTime());

    // Calculate the relative deformation gradient with the volumetric term
    // removed
    surfaceTensorField relFbar(1.0/cbrt(relJ)*relFf());

    // Update bE trial
    bEbarTrialf = transform(relFbar, bEbarf.oldTime());

    // Calculate trial deviatoric stress
    surfaceSymmTensorField sTrial(mu_*dev(bEbarTrialf));

    const surfaceScalarField Ibar(tr(bEbarTrialf)/3.0);
    const surfaceScalarField muBar(Ibar*mu_);

    // Check for plastic loading
    // and calculate increment of plastic equivalent strain
    // i.e. the plastic multiplier

    // Store previous iteration for under-relaxation and calculation of plastic
    // residual in the solver
    DEpsilonPfRef().storePrevIter();

    // Normalise residual in Newton method with respect to mag(bE)
    const scalar maxMagBE = max(gMax(mag(bEbarTrialf.primitiveField())), SMALL);

    // Trial yield function
    // sigmaY is the Cauchy yield stress so we scale it by J
    const surfaceScalarField fTrial
    (
        mag(sTrial) - sqrtTwoOverThree_*Jf*sigmaYf
    );

    // Calculate DLambdaf_ and plasticNf_
    forAll(fTrial, faceI)
    {
        setFaceValues(faceI);

        // Update plasticN, DLambda, DSigmaY and sigmaY for this face
        updatePlasticity
        (
            plasticNf[faceI],
            DLambdaf[faceI],
            sigmaYf[faceI],
            sigmaYfOld[faceI],
            fTrial[faceI],
            sTrial[faceI],
            epsilonPEqfOld[faceI],
            muBar[faceI],
            maxMagBE
        );
    }

    forAll(fTrial.boundaryField(), patchI)
    {
        // Take references to the boundary patch fields for efficiency
        const scalarField& pfTrial = fTrial.boundaryField()[patchI];
        const symmTensorField& psTrial = sTrial.boundaryField()[patchI];
        symmTensorField& pN = plasticNf.boundaryFieldRef()[patchI];
        scalarField& pDLambda = DLambdaf.boundaryFieldRef()[patchI];
        scalarField& psigmaY = sigmaYf.boundaryFieldRef()[patchI];
        const scalarField& pmuBar = muBar.boundaryField()[patchI];
        const scalarField& psigmaYOld = sigmaYfOld.boundaryField()[patchI];
        const scalarField& pepsilonPEqOld =
            epsilonPEqfOld.boundaryField()[patchI];

        forAll(pfTrial, faceI)
        {
            setSurfacePatchFaceValues(patchI, faceI);

            // Update plasticN, DLambda, DSigmaY and sigmaY for this face
            this->updatePlasticity
            (
                pN[faceI],
                pDLambda[faceI],
                psigmaY[faceI],
                psigmaYOld[faceI],
                pfTrial[faceI],
                psTrial[faceI],
                pepsilonPEqOld[faceI],
                pmuBar[faceI],
                maxMagBE
            );
        }
    }

    // Update DEpsilonP and DEpsilonPEq
    DEpsilonPEqfRef() = sqrtTwoOverThree_*DLambdaf;
    DEpsilonPfRef() = Ibar*DLambdaf*plasticNf;
    DEpsilonPfRef().relax();

    // Calculate deviatoric stress
    const surfaceSymmTensorField s(sTrial - 2*mu_*DEpsilonPf());

    // Update bEbar
    if (updateBEbarConsistent_)
    {
        const surfaceSymmTensorField devBEbar(s/mu_);
        bEbarf = devBEbar + this->Ibar(devBEbar)*I;
    }
    else
    {
        bEbarf = (s/mu_) + Ibar*I;
    }

    // Update the Cauchy stress
    // Note: updayeSigmaHyd is not implemented for surface fields
    sigma = (1.0/Jf)*(0.5*K_*(sqr(Jf) - 1)*I + s);

    epsilonPEqfRef() += DEpsilonPEqf();
    epsilonPfRef() += DEpsilonPf();
}


// ************************************************************************* //
