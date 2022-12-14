/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
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
    Foam::burstFvPatchFieldBase

Description

Usage

SourceFiles
    burstFvPatchFieldBase.C

\*---------------------------------------------------------------------------*/

#ifndef burstFvPatchFieldBase_H
#define burstFvPatchFieldBase_H

#include "fvPatchField.H"
#include "volFields.H"
#include "burstFvPatchBase.H"
#include "burstPolyPatchBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                    Class burstFvPatchFieldBase Declaration
\*---------------------------------------------------------------------------*/

class burstFvPatchFieldBase
{
protected:
    // Protected Data

        //- Reference to the burstPolyPatch
        burstFvPatchBase& burstFvPatch_;

        //- Reference to the burstPolyPatch
        const burstPolyPatchBase& burstPolyPatch_;

        //- Remove the intact and use the burst condition
        mutable bool unblock_;

        //- Force patch to close
        mutable bool block_;


public:

    //- Runtime type information
    TypeName("burstBase");


    // Constructor

        // Construct from fvPatch
        burstFvPatchFieldBase(const fvPatch&);


    // Destructor
    virtual ~burstFvPatchFieldBase();


    // Member Functions

        //- Set the unblock flag
        void unblock(const bool unblocked) const
        {
            unblock_ = unblocked;
            block_ = !unblock_;
        }

        //- Un set the unblock flag
        void block(const bool blocked) const
        {
            block_ = blocked;
            unblock_ = !block_;
        }

        //- Return the burst polyPatch base
        const burstFvPatchBase& burstFvPatch() const
        {
            return burstFvPatch_;
        }

        //- Return the burst polyPatch base
        const burstPolyPatchBase& burstPolyPatch() const
        {
            return burstPolyPatch_;
        }

        //- Return the intact field
        tmp<scalarField> intact() const
        {
            if (block_)
            {
                return tmp<scalarField>
                (
                    new scalarField(burstFvPatch_.patch().size(), 1.0)
                );
            }
            else if (unblock_)
            {
                return tmp<scalarField>
                (
                    new scalarField(burstFvPatch_.patch().size(), 0.0)
                );
            }
            return burstFvPatch_.intact();
        }

        //- Update the burst patch if necessary
        void update();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
