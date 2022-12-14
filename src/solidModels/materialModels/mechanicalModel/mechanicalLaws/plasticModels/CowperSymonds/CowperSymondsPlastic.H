/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     |
    \\  /    A nd           | For copyright notice see file Copyright
     \\/     M anipulation  |
-------------------------------------------------------------------------------
31-03-2022 Synthetik Applied Technologies: Added CowperSymondsPlastic
-------------------------------------------------------------------------------
License
    This file is a derivative work of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Class
    CowperSymondsPlastic

Description
    Cowper-Symonds plastic mechanical law.

SourceFiles
    CowperSymondsPlastic.C
    CowperSymondsPlastics.C


\*---------------------------------------------------------------------------*/

#ifndef CowperSymondsPlastic_H
#define CowperSymondsPlastic_H

#include "mechanicalLaw.H"
#include "plasticModel.H"
#include "UautoPtr.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         Class CowperSymondsPlastic Declaration
\*---------------------------------------------------------------------------*/

template<class PlasticType>
class CowperSymondsPlastic
:
    public PlasticType
{
    // Private data

        //- Const reference to temperature
        UautoPtr<const volScalarField> T_;

        //- Saved temporary interpolated temperature
        tmp<surfaceScalarField> Tf_;

        //- Effective elastic strain rate
        defineVolSurfaceFieldTypeFuncs
        (
            epsilonElasticEffDot,
            scalar,
            dimensionedScalar("epsilonElasticEffDot", inv(dimTime), Zero)
        );

        //- Material parameters
        dimensionedScalar C_;
        scalar p_;

        //- Strength hardening coefficient
        scalar n_;

        // Yield stress
        dimensionedScalar sigmaY0_;

        //- Is the total strain rate used
        Switch totalStrainRate_;

        //- Are temperature effects included
        Switch temperatureEffects_;

         //- Thermal softening coefficient
        dimensionedScalar m_;

        //- Reference temperature
        dimensionedScalar Tref_;

        //- Melting temperature
        dimensionedScalar Tm_;


        mutable scalar Ti_;
        mutable scalar eDoti_;
        mutable scalar sigmaYi_;


    // Private Member Functions

        //- Return the current Kirchhoff yield stress
        virtual scalar curYieldStress
        (
            const scalar curEpsilonPEq,     // Current equivalent plastic strain
            const scalar curEpsilonPEqDot,  // Current equivalent plastic strain rate
            const scalar J = 1.0            // Current Jacobian
        ) const;

        //- Set local values for fields not passed in functions
        virtual void setCellValues(const label celli) const;
        virtual void setVolPatchFaceValues
        (
            const label patchi,
            const label facei
        ) const;
        virtual void setFaceValues(const label facei) const;
        virtual void setSurfacePatchFaceValues
        (
            const label patchi,
            const label facei
        ) const;

        virtual bool nonLinearPlasticity() const
        {
            return true;
        }

        virtual dimensionedScalar sigmaY0() const
        {
            return sigmaY0_;
        }
public:


    // Constructors

        //- Construct from dictionary
        CowperSymondsPlastic
        (
            const word& name,
            const fvMesh& mesh,
            const dictionary& dict,
            const nonLinearGeometry::nonLinearType& nonLinGeom
        );


    // Destructor

        virtual ~CowperSymondsPlastic();


    // Member Functions

        //- Calculate the stress
        virtual void correct(volSymmTensorField& sigma);

        //- Calculate the stress
        virtual void correct(surfaceSymmTensorField& sigma);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#include "CowperSymondsPlastic.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
