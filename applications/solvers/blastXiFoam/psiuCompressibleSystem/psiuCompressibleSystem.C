/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019-2021
     \\/     M anipulation  | Synthetik Applied Technologies
-------------------------------------------------------------------------------
License
    This file is derivative work of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "psiuCompressibleSystem.H"
#include "fvm.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(psiuCompressibleSystem, 0);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::psiuCompressibleSystem::psiuCompressibleSystem
(
    const fvMesh& mesh
)
:
    compressibleSystem(mesh),
    thermo_(psiuReactionThermo::New(mesh)),
    rho_
    (
        IOobject
        (
            "rho",
            mesh.time().timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        thermo_->rho()
    ),
    p_(thermo_->p()),
    T_(thermo_->T()),
    e_(thermo_->he()),
    eu_(thermo_->heu()),
    rhoEu_
    (
        IOobject
        (
            "rhoEu",
            mesh.time().timeName(),
            mesh
        ),
        eu_*rho_
    )
{
    thermo_->validate("psiuCompressibleSystem", "ea");

    if (min(thermo_->mu()).value() > small)
    {
        turbulence_ =
            compressible::momentumTransportModel::New
            (
                rho_,
                U_,
                rhoPhi_,
                thermo_()
            );
        thermophysicalTransport_ =
            fluidThermophysicalTransportModel::New
            (
                turbulence_(),
                thermo_()
            );
    }

    fluxScheme_ = fluxScheme::NewSingle(mesh);
    encode();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::psiuCompressibleSystem::~psiuCompressibleSystem()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::psiuCompressibleSystem::solve()
{
    volScalarField deltaRho(fvc::div(rhoPhi_));
    volVectorField deltaRhoU(fvc::div(rhoUPhi_) - g_*rho_);
    volScalarField deltaRhoE
    (
        fvc::div(rhoEPhi_)
      - (rhoU_ & g_)
    );
    volScalarField deltaRhoEu
    (
        fvc::div(fluxScheme_->energyFlux(rho_, U_, eu_, p_))
      - (rhoU_ & g_)
    );

    //- Store changed in mass, momentum and energy
    this->storeAndBlendDelta(deltaRho);
    this->storeAndBlendDelta(deltaRhoU);
    this->storeAndBlendDelta(deltaRhoE);
    this->storeAndBlendDelta(deltaRhoEu);

    //- Store old values
    this->storeAndBlendOld(rho_);
    this->storeAndBlendOld(rhoU_);
    this->storeAndBlendOld(rhoE_);
    this->storeAndBlendOld(rhoEu_);

    dimensionedScalar dT = rho_.time().deltaT();
    rho_ -= dT*deltaRho;
    rho_.correctBoundaryConditions();

    vector solutionDs((vector(rho_.mesh().solutionD()) + vector::one)/2.0);
    rhoU_ -= cmptMultiply(dT*deltaRhoU, solutionDs);
    rhoE_ -= dT*deltaRhoE;
    rhoEu_ -= dT*deltaRhoEu;
}


void Foam::psiuCompressibleSystem::postUpdate()
{
    if (!turbulence_.valid())
    {
        return;
    }

    this->decode();

   // Solve momentum diffusion
    fvVectorMatrix UEqn
    (
        fvm::ddt(rho_, U_) - fvc::ddt(rho_, U_)
      + turbulence_->divDevTau(U_)
    );
    volScalarField dTDivSigmaDotU
    (
        rho_.mesh().time().deltaT()
       *fvc::div
        (
            fvc::dotInterpolate(rho_.mesh().Sf(), turbulence_->devTau())
          & fluxScheme_->Uf()
        )
    );
    rhoE_ += dTDivSigmaDotU;
    rhoEu_ += dTDivSigmaDotU;

    UEqn.solve();
    rhoU_ = rho_*U_;

    // Solve thermal energy diffusion
    e_ = rhoE_/rho_ - 0.5*magSqr(U_);
    eu_ = rhoEu_/rho_ - 0.5*magSqr(U_);
    Foam::solve
    (
        fvm::ddt(rho_, e_) - fvc::ddt(rho_, e_)
      - fvm::laplacian(thermophysicalTransport_->alphaEff(), e_)
    );
    Foam::solve
    (
        fvm::ddt(rho_, eu_) - fvc::ddt(rho_, eu_)
      - fvm::laplacian(thermophysicalTransport_->alphaEff(), eu_)
    );

    rhoE_ = rho_*(e_ + 0.5*magSqr(U_));
    rhoEu_ = rho_*(eu_ + 0.5*magSqr(U_));

    turbulence_->correct();

//     if(temperatureFix)
    {
        scalar Tulow = 250.0;
        volScalarField dummyTu(thermo_->Tu());
        dummyTu.min(Tulow);

        eu_ = max(eu_, thermo_->he(p_, dummyTu));
        rhoEu_ = rho_*(eu_ + 0.5*magSqr(U_));
    }

    thermo_->correct();
    p_.ref() = rho_/thermo_->psi();
    p_.correctBoundaryConditions();
    rho_.boundaryFieldRef() ==
        thermo_->psi().boundaryField()*p_.boundaryField();
}


void Foam::psiuCompressibleSystem::update()
{
    fluxScheme_->update
    (
        rho_,
        U_,
        e_,
        p_,
        speedOfSound()(),
        phi_,
        rhoPhi_,
        rhoUPhi_,
        rhoEPhi_
    );
}


void Foam::psiuCompressibleSystem::decode()
{
    U_.ref() = rhoU_()/rho_();
    U_.correctBoundaryConditions();

    rhoU_.boundaryFieldRef() = rho_.boundaryField()*U_.boundaryField();

    volScalarField E(rhoE_/rho_);
    e_.ref() = E() - 0.5*magSqr(U_());
    e_.correctBoundaryConditions();

    volScalarField Eu(rhoEu_/rho_);
    eu_.ref() = Eu() - 0.5*magSqr(U_());
    eu_.correctBoundaryConditions();

    rhoE_.boundaryFieldRef() =
        rho_.boundaryField()
       *(
            e_.boundaryField()
          + 0.5*magSqr(U_.boundaryField())
        );
    rhoEu_.boundaryFieldRef() =
        rho_.boundaryField()
       *(
            eu_.boundaryField()
          + 0.5*magSqr(U_.boundaryField())
        );

    thermo_->correct();
    p_.ref() = rho_/thermo_->psi();
    p_.correctBoundaryConditions();
    rho_.boundaryFieldRef() ==
        thermo_->psi().boundaryField()*p_.boundaryField();
}


void Foam::psiuCompressibleSystem::encode()
{
    rhoU_ = rho_*U_;
    rhoE_ = rho_*(e_ + 0.5*magSqr(U_));
    rhoEu_ = rho_*(eu_ + 0.5*magSqr(U_));
}


Foam::tmp<Foam::volScalarField>
Foam::psiuCompressibleSystem::speedOfSound() const
{
    return sqrt(thermo_->gamma()/thermo_->psi());
}


const Foam::volScalarField& Foam::psiuCompressibleSystem::rho() const
{
    return rho_;
}


Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::rhou() const
{
    return thermo_->rhou();
}


Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::Cv() const
{
    return thermo_->Cv();
}


Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::mu() const
{
    return thermo_->mu();
}


Foam::tmp<Foam::scalarField>
Foam::psiuCompressibleSystem::mu(const label patchi) const
{
    return thermo_->mu(patchi);
}


Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::muu() const
{
    return thermo_->muu();
}


Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::mub() const
{
    return thermo_->mub();
}


Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::nu() const
{
    return thermo_->nu();
}

Foam::tmp<Foam::scalarField>
Foam::psiuCompressibleSystem::nu(const label patchi) const
{
    return thermo_->nu(patchi);
}

Foam::tmp<Foam::volScalarField>
Foam::psiuCompressibleSystem::alpha() const
{
    return thermo_->alpha();
}

Foam::tmp<Foam::scalarField>
Foam::psiuCompressibleSystem::alpha(const label patchi) const
{
    return thermo_->alpha(patchi);
}

Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::alphaEff
(
    const volScalarField& alphat
) const
{
    return thermo_->alphaEff(alphat);
}

Foam::tmp<Foam::scalarField> Foam::psiuCompressibleSystem::alphaEff
(
    const scalarField& alphat,
    const label patchi
) const
{
    return thermo_->alphaEff(alphat, patchi);
}

Foam::tmp<Foam::volScalarField>
Foam::psiuCompressibleSystem::alphahe() const
{
    return thermo_->alphahe();
}

Foam::tmp<Foam::scalarField>
Foam::psiuCompressibleSystem::alphahe(const label patchi) const
{
    return thermo_->alphahe(patchi);
}

Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::kappa() const
{
    return thermo_->kappa();
}

Foam::tmp<Foam::scalarField>
Foam::psiuCompressibleSystem::kappa(const label patchi) const
{
    return thermo_->kappa(patchi);
}

Foam::tmp<Foam::volScalarField> Foam::psiuCompressibleSystem::kappaEff
(
    const volScalarField& alphat
) const
{
    return thermo_->kappaEff(alphat);
}

Foam::tmp<Foam::scalarField> Foam::psiuCompressibleSystem::kappaEff
(
    const scalarField& alphat,
    const label patchi
) const
{
    return thermo_->kappaEff(alphat, patchi);
}
// ************************************************************************* //
