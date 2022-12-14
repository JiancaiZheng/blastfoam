/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2021 OpenFOAM Foundation
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

Class
    Foam::burstCyclicAMIPointPatch

Description
    Cyclic patch for post-processing.

SourceFiles
    burstCyclicAMIPointPatch.C

\*---------------------------------------------------------------------------*/

#ifndef burstCyclicAMIPointPatch_H
#define burstCyclicAMIPointPatch_H

#include "cyclicAMIPointPatch.H"
#include "burstCyclicAMIPolyPatch.H"
#include "pointBoundaryMesh.H"
#include "valuePointPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class burstCyclicAMIPointPatch Declaration
\*---------------------------------------------------------------------------*/

class burstCyclicAMIPointPatch
:
    public cyclicAMIPointPatch
{
    // Private Data

        //- Const reference to the burstCyclicAMIPolyPatch
        const burstCyclicAMIPolyPatch& burstCyclicAMIPolyPatch_;


    // Private Member Functions

        //- Disallow default bitwise copy construction
        burstCyclicAMIPointPatch(const burstCyclicAMIPointPatch&);

        //- Disallow default bitwise assignment
        void operator=(const burstCyclicAMIPointPatch&);


    // Demand driven private data

        //- Initialise the calculation of the patch geometry
        virtual void initCalcGeometry(PstreamBuffers&);

        //- Calculate the patch geometry
        virtual void calcGeometry(PstreamBuffers&);

        //- Initialise the patches for moving points
        virtual void initMovePoints(PstreamBuffers&, const pointField&);

        //- Correct patches after moving points
        virtual void movePoints(PstreamBuffers&, const pointField&);

        //- Initialise the update of the patch topology
        virtual void initUpdateMesh(PstreamBuffers&);

        //- Update of the patch topology
        virtual void updateMesh(PstreamBuffers&);


public:

    //- Runtime type information
    TypeName(burstCyclicAMIPolyPatch::typeName_());


    // Constructors

        //- Construct from components
        burstCyclicAMIPointPatch
        (
            const polyPatch& patch,
            const pointBoundaryMesh& bm
        );


    //- Destructor
    virtual ~burstCyclicAMIPointPatch();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
