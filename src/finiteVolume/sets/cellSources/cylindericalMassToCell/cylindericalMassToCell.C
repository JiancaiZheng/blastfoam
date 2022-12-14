/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021
     \\/     M anipulation  | Synthetik Applied Technologies
-------------------------------------------------------------------------------
License
    This file is a derivative work of OpenFOAM.

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

#include "cylindericalMassToCell.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(cylindericalMassToCell, 0);
    addToRunTimeSelectionTable(topoSetSource, cylindericalMassToCell, word);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::cylindericalMassToCell::cylindericalMassToCell
(
    const polyMesh& mesh,
    const dictionary& dict
)
:
    massToCell(dict),
    cylinderToCell
    (
        mesh,
        p1(dict),
        p2(dict),
        R(dict)
    )
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::cylindericalMassToCell::~cylindericalMassToCell()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::cylindericalMassToCell::applyToSet
(
    const topoSetSource::setAction action,
    topoSet& set
) const
{
    labelHashSet oldSet(set);
    cylinderToCell::applyToSet(action, set);

    checkMass
    (
        action == topoSetSource::REMOVE
      ? oldSet ^ set
      : (action == topoSetSource::ADD ? set ^ oldSet : set),
        mesh_
    );
}


// ************************************************************************* //
