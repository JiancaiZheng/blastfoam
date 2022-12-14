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
    Foam::coupledPointMotionFvPatchField

Description
    Foam::globalMappedPointPatchField

SourceFiles
    globalMappedPointPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef globalMappedPointPatchField_H
#define globalMappedPointPatchField_H

#include "Random.H"
#include "fixedValuePointPatchFields.H"
#include "globalPolyBoundaryMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Class globalMappedPointPatchField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class globalMappedPointPatchField
:
    public fixedValuePointPatchField<Type>
{
    // Private Data

        //- Reference to the global boundaryMesh
        const globalPolyBoundaryMesh& globalBoundary_;

        //- Name of neighbour displacement field
        const word nbrName_;


public:

    //- Runtime type information
    TypeName("globalMapped");


    // Constructors

        //- Construct from patch and internal field
        globalMappedPointPatchField
        (
            const pointPatch&,
            const DimensionedField<Type, pointMesh>&
        );

        //- Construct from patch, internal field and dictionary
        globalMappedPointPatchField
        (
            const pointPatch&,
            const DimensionedField<Type, pointMesh>&,
            const dictionary&
        );

        //- Construct by mapping given globalMappedPointPatchField
        //  onto a new patch
        globalMappedPointPatchField
        (
            const globalMappedPointPatchField<Type>&,
            const pointPatch&,
            const DimensionedField<Type, pointMesh>&,
            const pointPatchFieldMapper&
        );

        //- Copy constructor
        globalMappedPointPatchField
        (
            const globalMappedPointPatchField<Type>&
        ) = delete;

        //- Copy constructor setting internal field reference
        globalMappedPointPatchField
        (
            const globalMappedPointPatchField<Type>&,
            const DimensionedField<Type, pointMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual autoPtr<pointPatchField<Type>> clone
        (
            const DimensionedField<Type, pointMesh>& iF
        ) const
        {
            return autoPtr<pointPatchField<Type>>
            (
                new globalMappedPointPatchField<Type>(*this, iF)
            );
        }


    // Member Functions

        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "globalMappedPointPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
