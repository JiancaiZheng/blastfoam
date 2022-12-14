/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019-2021
     \\/     M anipulation  | Synthetik Applied Technologies
-------------------------------------------------------------------------------
License
    This file is a derived work of OpenFOAM.

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
    Foam::fv::RiemannConvectionScheme

Description
    Comvection scheme that uses explicit fluxes for both fvc and fvm based
    convection terms. The fluxes are calculated using the saved Riemann solver
    fields in with interpolation from upwinding and downwinding directions

SourceFiles
    RiemannConvectionScheme.C

\*---------------------------------------------------------------------------*/

#ifndef RiemannConvectionScheme_H
#define RiemannConvectionScheme_H

#include "convectionScheme.H"
#include "fluxSchemeBase.H"
#include "fluxScheme.H"
#include "phaseFluxScheme.H"
#include "UautoPtr.H"
#include "hashedWordList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace fv
{

/*---------------------------------------------------------------------------*\
                       Class RiemannConvectionScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class RiemannConvectionScheme
:
    public fv::convectionScheme<Type>
{
    // Private Data

        //- Reference to flux scheme
        UautoPtr<const fluxSchemeBase> fluxSchemePtr_;


public:

    //- Runtime type information
    TypeName("Riemann");


    // Constructors

        //- Construct from flux and interpolation scheme
        RiemannConvectionScheme
        (
            const fvMesh& mesh,
            const surfaceScalarField& faceFlux,
            const tmp<surfaceInterpolationScheme<Type>>& scheme
        )
        :
            convectionScheme<Type>(mesh, faceFlux),
            fluxSchemePtr_(nullptr)
        {
            const word phaseFluxName
            (
                IOobject::groupName(fluxSchemeBase::typeName, faceFlux.group())
            );
            if (mesh.foundObject<fluxScheme>(fluxSchemeBase::typeName))
            {
                fluxSchemePtr_.set
                (
                    &mesh.lookupObject<fluxScheme>(fluxSchemeBase::typeName)
                );
            }
            else if (mesh.foundObject<phaseFluxScheme>(phaseFluxName))
            {
                fluxSchemePtr_.set
                (
                    &mesh.lookupObject<phaseFluxScheme>(phaseFluxName)
                );
            }
            else
            {
                // If the phase name is not the primary phase
                // check all of the included phases
                HashTable<const phaseFluxScheme*> schemes
                (
                    mesh.lookupClass<phaseFluxScheme>()
                );
                forAllConstIter
                (
                    HashTable<const phaseFluxScheme*>,
                    schemes,
                    iter
                )
                {
                    hashedWordList phases(iter()->phases());
                    {
                        if (phases.found(faceFlux.group()))
                        {
                            fluxSchemePtr_.set(iter());
                        }
                    }
                }
            }
            if (!fluxSchemePtr_.valid())
            {
                FatalErrorInFunction
                    << "Could not determine a fluxScheme to use"
                    << abort(FatalError);
            }
        }


        //- Construct from flux and Istream
        RiemannConvectionScheme
        (
            const fvMesh& mesh,
            const surfaceScalarField& faceFlux,
            Istream& is
        )
        :
            convectionScheme<Type>(mesh, faceFlux),
            fluxSchemePtr_(nullptr)
        {
            const word phaseFluxName
            (
                IOobject::groupName(fluxSchemeBase::typeName, faceFlux.group())
            );
            if (mesh.foundObject<fluxScheme>(fluxSchemeBase::typeName))
            {
                fluxSchemePtr_.set
                (
                    &mesh.lookupObject<fluxScheme>(fluxSchemeBase::typeName)
                );
            }
            else if (mesh.foundObject<phaseFluxScheme>(phaseFluxName))
            {
                fluxSchemePtr_.set
                (
                    &mesh.lookupObject<phaseFluxScheme>(phaseFluxName)
                );
            }
            else
            {
                // If the phase name is not the primary phase
                // check all of the included phases
                HashTable<const phaseFluxScheme*> schemes
                (
                    mesh.lookupClass<phaseFluxScheme>()
                );
                forAllConstIter
                (
                    HashTable<const phaseFluxScheme*>,
                    schemes,
                    iter
                )
                {
                    hashedWordList phases(iter()->phases());
                    {
                        if (phases.found(faceFlux.group()))
                        {
                            fluxSchemePtr_.set(iter());
                        }
                    }
                }
            }
            if (!fluxSchemePtr_.valid())
            {
                FatalErrorInFunction
                    << "Could not determine a fluxScheme to use"
                    << abort(FatalError);
            }
        }

        //- Disallow default bitwise copy construction
        RiemannConvectionScheme(const RiemannConvectionScheme&) = delete;


    // Member Functions

        tmp<GeometricField<Type, fvsPatchField, surfaceMesh>> interpolate
        (
            const surfaceScalarField&,
            const GeometricField<Type, fvPatchField, volMesh>&
        ) const;

        tmp<GeometricField<Type, fvsPatchField, surfaceMesh>> flux
        (
            const surfaceScalarField&,
            const GeometricField<Type, fvPatchField, volMesh>&
        ) const;

        tmp<fvMatrix<Type>> fvmDiv
        (
            const surfaceScalarField&,
            const GeometricField<Type, fvPatchField, volMesh>&
        ) const;

        tmp<GeometricField<Type, fvPatchField, volMesh>> fvcDiv
        (
            const surfaceScalarField&,
            const GeometricField<Type, fvPatchField, volMesh>&
        ) const;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const RiemannConvectionScheme&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "RiemannConvectionScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
