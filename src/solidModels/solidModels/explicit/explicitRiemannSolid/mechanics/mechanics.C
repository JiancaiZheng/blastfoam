/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

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

#include "mechanics.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(mechanics, 0);


// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

mechanics::mechanics
(
    const volTensorField& F,
    const operations& ops
)
:
    MeshObject<fvMesh, MoveableMeshObject, mechanics>(F.mesh()),
    mesh_(F.mesh()),

    ops_(ops),

    F_(F),

    invF_
    (
        IOobject
        (
            "invF",
            mesh_.time().timeName(),
            mesh_
        ),
        inv(F_)
    ),

    J_
    (
        IOobject
        (
            "J",
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        det(F_)
    ),

    N_("N", mesh_.Sf()/mesh_.magSf()),

    n_
    (
        IOobject
        (
            "n",
            mesh_.time().timeName(),
            mesh_
        ),
        mesh_.Sf()/mesh_.magSf()
    ),

    stabRhoU_
    (
        IOobject
        (
            "stabRhoU",
            mesh_.time().timeName(),
            mesh_
        ),
        mesh_,
        dimensionedTensor("0", dimVelocity, Zero)
    ),

    stabTraction_
    (
        IOobject
        (
            "stabTraction",
            mesh_.time().timeName(),
            mesh_
        ),
        mesh_,
        dimensionedTensor("0", inv(dimVelocity), Zero)
    ),

    stretch_
    (
        IOobject
        (
            "stretch",
            mesh_.time().timeName(),
            mesh_
        ),
        mesh_,
        dimensionedScalar("0", dimless, 1.0)
    )

{}


// * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * * //

mechanics::~mechanics()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


bool mechanics::movePoints()
{
    N_ = mesh_.Sf()/mesh_.magSf();
    return true;
}

void mechanics::correctN()
{
    surfaceTensorField invFf(fvc::interpolate(invF_));
    n_ = (invFf.T() & N_)/(mag(invFf.T() & N_));
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void mechanics::correctDeformation(const bool useOldTime)
{
    // Spatial normals
    correctN();

    J_ = det(F_);
    invF_ = inv(F_);

    // Stretch
    volTensorField C(F_.T() & F_);
    forAll(C, celli)
    {
        ops_.eigenStructure(C[celli]);
        stretch_[celli] = sqrt(cmptMin(ops_.eigenValue()));
    }
    const volTensorField::Boundary& pC(C.boundaryField());
    volScalarField::Boundary& pstretch = stretch_.boundaryFieldRef();
    forAll(pC, patchi)
    {
        forAll(pC[patchi], facei)
        {
            ops_.eigenStructure(pC[patchi][facei]);
            pstretch[patchi][facei] = sqrt(cmptMin(ops_.eigenValue()));
        }
    }
}


void mechanics::correct
(
    const volScalarField& pWaveSpeed,
    const volScalarField& sWaveSpeed
)
{
    // Stabilisation matrices
    surfaceTensorField nn(n_*n_);
    stabRhoU_ =
        fvc::interpolate(pWaveSpeed)*nn
      + fvc::interpolate(sWaveSpeed)*(I - nn);

    stabTraction_ =
        nn/fvc::interpolate(pWaveSpeed)
      + (I - nn)/fvc::interpolate(sWaveSpeed);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
