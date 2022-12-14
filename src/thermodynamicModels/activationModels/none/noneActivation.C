/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019 Synthetik Applied Technologies
     \\/     M anipulation  |
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

#include "noneActivation.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace activationModels
{
    defineTypeNameAndDebug(noneActivation, 0);
    addToRunTimeSelectionTable(activationModel, noneActivation, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::activationModels::noneActivation::noneActivation
(
    const fvMesh& mesh,
    const dictionary& dict,
    const word& phaseName
)
:
    activationModel(mesh, dict, phaseName, false),
    setLambda_(true)
{
    if (detonationPoints_.size() == 0)
    {
        detonationPoints_.resize(1);
        detonationPoints_.set
        (
            0,
            new detonationPoint
            (
                returnReduce
                (
                    minMagSqr(mesh.C().primitiveField()),
                    minMagSqrOp<vector>()
                ),
                0.0,
                0.0
            )
        );
    }
    forAll(detonationPoints_, i)
    {
        if (!detonationPoints_[i].activated())
        {
            detonationPoints_[i].activated() = true;
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::activationModels::noneActivation::~noneActivation()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::activationModels::noneActivation::solve()
{
    if (setLambda_)
    {
        lambda_ == 1.0;
        setLambda_ = false;
    }
}


Foam::tmp<Foam::volScalarField>
Foam::activationModels::noneActivation::ddtLambda() const
{
    return volScalarField::New
    (
        "noActivation:ddtLambda",
        lambda_.mesh(),
        dimensionedScalar("0", inv(dimTime), 0.0)
    );
}


Foam::tmp<Foam::volScalarField>
Foam::activationModels::noneActivation::initESource() const
{
    return volScalarField::New
    (
        "noActivation:initESource",
        e0_*(1.0 - lambda_)
    );
}


Foam::tmp<Foam::volScalarField>
Foam::activationModels::noneActivation::ESource() const
{
    return volScalarField::New
    (
        "noActivation:ESource",
        lambda_.mesh(),
        dimensionedScalar("0", e0_.dimensions()/dimTime, 0.0)
    );
}


// ************************************************************************* //
