/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2022
     \\/     M anipulation  | Synthetik Applied Technologies
-------------------------------------------------------------------------------
License
    This file is a derivative work of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Class
    totalLagSolid

Description
    Base class for total Lagrangian solid models

SourceFiles
    totalLagSolid.C

\*---------------------------------------------------------------------------*/

#ifndef totalLagSolid_H
#define totalLagSolid_H

#include "solidModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace solidModels
{

/*---------------------------------------------------------------------------*\
                        Class totalLagSolid Declaration
\*---------------------------------------------------------------------------*/

template<class IncrementalModel>
class totalLagSolid
:
    public IncrementalModel
{
protected:
    // Protected data

        //- Total deformation gradient
        volTensorField F_;

        //- Relative deformation gradient
        volTensorField relF_;

        //- Inverse of the total deformation gradient
        volTensorField Finv_;

        //- Relative Jacobian: Jacobian of the relative deformation gradient
        volTensorField relFinv_;

        //- Jacobian of total deformation gradient
        volScalarField J_;

        //- Relative Jacobian: Jacobian of the relative deformation gradient
        volScalarField relJ_;

        //- Implicit stiffness; coefficient of the Laplacian term
        //  The value of this term only affects convergence and not the
        //  answer. In the case of Hookean elasticity, (2*mu + lambda)
        //  is the optimum
        const volScalarField impK_;

        //- Implicit stiffness surface field
        surfaceScalarField impKf_;


    // Protected member functions

        //- Update fields
        virtual void update(const bool correctSigma = true);

        //- Return nonlinear geometry enumerator
        virtual nonLinearGeometry::nonLinearType nonLinGeom() const
        {
            return nonLinearGeometry::TOTAL_LAGRANGIAN;
        }


public:

    // Constructors

        //- Construct from components
        totalLagSolid
        (
            const word& type,
            dynamicFvMesh& mesh,
            const bool isSolid = true
        );

    // Destructor

        virtual ~totalLagSolid()
        {}


    // Member Functions

        //- Traction boundary surface normal gradient
        virtual tmp<vectorField> tractionBoundarySnGrad
        (
            const vectorField& traction,
            const scalarField& pressure,
            const fvPatch& patch
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace solidModel
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#include "totalLagSolid.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
