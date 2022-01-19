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
    Foam::burstCyclicPolyPatch

Description
    Poly patch for a burstCyclicing surface (cannot be undone)

SourceFiles
    burstCyclicPolyPatch.C

\*---------------------------------------------------------------------------*/

#ifndef burstCyclicPolyPatch_H
#define burstCyclicPolyPatch_H

#include "cyclicPolyPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class burstCyclicPolyPatch Declaration
\*---------------------------------------------------------------------------*/

class burstCyclicPolyPatch
:
    public cyclicPolyPatch
{
    // Private Data

        //- Intact field
        scalarField intact_;

        //- Is pressure used
        bool usePressure_;

        //- Burst pressure
        scalar pBurst_;

        //- bool impulse used
        bool useImpulse_;

        //- Burst impulse
        scalar impulseBurst_;

        //- Do individual faces burstCyclic
        bool partialBurst_;

        //- Is mapping needed
        //  Since there is no mapping within polyPatch we will handle this
        //  on the first call from a geometric field
        mutable bool needMap_;


protected:

    // Protected Member functions

        //- Initialise the calculation of the patch geometry
        virtual void initCalcGeometry(PstreamBuffers&);

        //- Initialise the calculation of the patch geometry
        virtual void initCalcGeometry
        (
            const primitivePatch& referPatch,
            pointField& nbrCtrs,
            vectorField& nbrAreas,
            pointField& nbrCc
        );

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

    //- Declare friendship with processorCyclicPolyPatch
    friend class processorCyclicPolyPatch;


    //- Runtime type information
    TypeName("burstCyclic");


    // Constructors

        //- Construct from components
        burstCyclicPolyPatch
        (
            const word& name,
            const label size,
            const label start,
            const label index,
            const polyBoundaryMesh& bm,
            const word& patchType
        );

        //- Construct from components
        burstCyclicPolyPatch
        (
            const word& name,
            const label size,
            const label start,
            const label index,
            const polyBoundaryMesh& bm,
            const word& patchType,
            const word& nbrPatchName
        );

        //- Construct from dictionary
        burstCyclicPolyPatch
        (
            const word& name,
            const dictionary& dict,
            const label index,
            const polyBoundaryMesh& bm,
            const word& patchType
        );

        //- Construct as copy, resetting the boundary mesh
        burstCyclicPolyPatch(const burstCyclicPolyPatch&, const polyBoundaryMesh&);

        //- Construct given the original patch and resetting the
        //  face list and boundary mesh information
        burstCyclicPolyPatch
        (
            const burstCyclicPolyPatch& pp,
            const polyBoundaryMesh& bm,
            const label index,
            const label newSize,
            const label newStart,
            const word& nbrPatchName
        );

        //- Construct given the original patch and a map
        burstCyclicPolyPatch
        (
            const burstCyclicPolyPatch& pp,
            const polyBoundaryMesh& bm,
            const label index,
            const labelUList& mapAddressing,
            const label newStart
        );

        //- Construct and return a clone, resetting the boundary mesh
        virtual autoPtr<polyPatch> clone(const polyBoundaryMesh& bm) const
        {
            return autoPtr<polyPatch>(new burstCyclicPolyPatch(*this, bm));
        }

        //- Construct and return a clone, resetting the face list
        //  and boundary mesh
        virtual autoPtr<polyPatch> clone
        (
            const polyBoundaryMesh& bm,
            const label index,
            const label newSize,
            const label newStart
        ) const
        {
            return autoPtr<polyPatch>
            (
                new burstCyclicPolyPatch
                (
                    *this,
                    bm,
                    index,
                    newSize,
                    newStart,
                    word::null
                )
            );
        }

        //- Construct and return a clone, resetting the face list
        //  and boundary mesh
        virtual autoPtr<polyPatch> clone
        (
            const polyBoundaryMesh& bm,
            const label index,
            const labelUList& mapAddressing,
            const label newStart
        ) const
        {
            return autoPtr<polyPatch>
            (
                new burstCyclicPolyPatch(*this, bm, index, mapAddressing, newStart)
            );
        }


    //- Destructor
    virtual ~burstCyclicPolyPatch();


    // Member Functions

        // Access

            //- Return the intact field
            const Field<scalar>& intact() const
            {
                return intact_;
            }

            //- Is mapping needed
            bool needMap() const;

            //- Return if pressure is used
            bool usePressure() const
            {
                return usePressure_;
            }

            //- Return burstCyclic pressure
            scalar pBurst() const
            {
                return pBurst_;
            }

            //- Return if impulse is used
            bool useImpulse() const
            {
                return useImpulse_;
            }

            //- Return burstCyclic impulse
            scalar impulseBurst() const
            {
                return impulseBurst_;
            }

            //- Return if individual faces burstCyclic
            bool partialBurst() const
            {
                return partialBurst_;
            }

        //- Write the polyPatch data as a dictionary
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //