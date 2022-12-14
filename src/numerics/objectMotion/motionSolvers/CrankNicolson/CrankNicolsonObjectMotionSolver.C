/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2015-2018 OpenFOAM Foundation
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

#include "CrankNicolsonObjectMotionSolver.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace objectMotionSolvers
{
    defineTypeNameAndDebug(CrankNicolson, 0);
    addToRunTimeSelectionTable(objectMotionSolver, CrankNicolson, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::objectMotionSolvers::CrankNicolson::CrankNicolson
(
    const CrankNicolson& solver,
    movingObject& body,
    objectMotionState& state,
    objectMotionState& state0
)
:
    objectMotionSolver(body, state, state0),
    aoc_(solver.aoc_),
    voc_(solver.voc_)
{}


Foam::objectMotionSolvers::CrankNicolson::CrankNicolson
(
    const dictionary& dict,
    movingObject& body,
    objectMotionState& state,
    objectMotionState& state0
)
:
    objectMotionSolver(body, state, state0),
    aoc_(dict.lookupOrDefault<scalar>("aoc", 0.5)),
    voc_(dict.lookupOrDefault<scalar>("voc", 0.5))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::objectMotionSolvers::CrankNicolson::~CrankNicolson()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::objectMotionSolvers::CrankNicolson::solve
(
    bool firstIter,
    const vector& fGlobal,
    const vector& tauGlobal,
    scalar deltaT,
    scalar deltaT0
)
{
    // Update the linear acceleration and torque
    updateAcceleration(fGlobal, tauGlobal);

    // Correct linear velocity
    v() = tConstraints()
      & (v0() + deltaT*(aoc_*a() + (1 - aoc_)*a0()));

    // Correct angular momentum
    pi() = rConstraints()
      & (pi0() + deltaT*(aoc_*tau() + (1 - aoc_)*tau0()));

    // Correct position
    centreOfRotation() =
        centreOfRotation0() + deltaT*(voc_*v() + (1 - voc_)*v0());

    // Correct orientation
    Tuple2<tensor, vector> Qpi =
        rotate(Q0(), (voc_*pi() + (1 - voc_)*pi0()), deltaT);
    Q() = Qpi.first();
}


// ************************************************************************* //
