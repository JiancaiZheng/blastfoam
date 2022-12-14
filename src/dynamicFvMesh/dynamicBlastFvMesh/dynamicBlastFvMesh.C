/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
21-05-2020  21-05-2020 Synthetik Applied Technologies: | Made the
                                dynamicMeshDict runtime modifiable
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

#include "dynamicBlastFvMesh.H"
#include "volFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(dynamicBlastFvMesh, 0);
}


// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

Foam::IOobject Foam::dynamicBlastFvMesh::dynamicMeshDictIOobject
(
    const IOobject& io
)
{
    IOobject dictHeader
    (
        "dynamicMeshDict",
        io.time().constant(),
        (io.name() == polyMesh::defaultRegion ? "" : io.name()),
        io.db(),
        IOobject::READ_IF_PRESENT,
        IOobject::NO_WRITE,
        false
    );

    // defaultRegion (region0) gets loaded from constant, other ones get loaded
    // from constant/<regionname>. Normally we'd use polyMesh::dbDir() but we
    // haven't got a polyMesh yet ...
    return IOobject
    (
        "dynamicMeshDict",
        io.time().constant(),
        (io.name() == polyMesh::defaultRegion ? "" : io.name()),
        io.db(),
        (
            dictHeader.typeHeaderOk<IOdictionary>(true)
          ? IOobject::MUST_READ_IF_MODIFIED
          : IOobject::NO_READ
        ),
        IOobject::NO_WRITE
    );
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dynamicBlastFvMesh::dynamicBlastFvMesh(const IOobject& io)
:
    dynamicFvMesh(io),
    dynamicMeshDict_(dynamicMeshDictIOobject(io))
{}


Foam::dynamicBlastFvMesh::dynamicBlastFvMesh
(
    const IOobject& io,
    pointField&& points,
    faceList&& faces,
    labelList&& allOwner,
    labelList&& allNeighbour,
    const bool syncPar
)
:
    dynamicFvMesh
    (
        io,
        move(points),
        move(faces),
        move(allOwner),
        move(allNeighbour),
        syncPar
    ),
    dynamicMeshDict_(dynamicMeshDictIOobject(io))
{}


Foam::dynamicBlastFvMesh::dynamicBlastFvMesh
(
    const IOobject& io,
    pointField&& points,
    faceList&& faces,
    cellList&& cells,
    const bool syncPar
)
:
    dynamicFvMesh
    (
        io,
        move(points),
        move(faces),
        move(cells),
        syncPar
    ),
    dynamicMeshDict_(dynamicMeshDictIOobject(io))
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::dynamicBlastFvMesh::~dynamicBlastFvMesh()
{}

// * * * * * * * * * * * * *  Public Static Functions  * * * * * * * * * * * //

bool Foam::refineMesh(dynamicFvMesh& mesh)
{
    if (isA<dynamicBlastFvMesh>(mesh))
    {
        return dynamicCast<dynamicBlastFvMesh>(mesh).refine();
    }
    return false;
}

// ************************************************************************* //
