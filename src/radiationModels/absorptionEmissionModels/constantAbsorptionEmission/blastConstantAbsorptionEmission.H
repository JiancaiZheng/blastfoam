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
    Foam::radiationModels::absorptionEmissionModels::blastConstant

Description
    Constant radiation absorption and emission coefficients for continuous
    phase

SourceFiles
    blastConstantAbsorptionEmission.C

\*---------------------------------------------------------------------------*/

#ifndef blastConstantAbsorptionEmission_H
#define blastConstantAbsorptionEmission_H

#include "blastAbsorptionEmissionModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace radiationModels
{
namespace absorptionEmissionModels
{

/*---------------------------------------------------------------------------*\
                          Class blastConstant Declaration
\*---------------------------------------------------------------------------*/

class blastConstant
:
    public blastAbsorptionEmissionModel
{
    // Private Data

        //- Absorption model dictionary
        dictionary coeffsDict_;

        //- Absorption coefficient / [1/m]
        dimensionedScalar a_;

        //- Emission coefficient / [1/m]
        dimensionedScalar e_;

        //- Emission contribution / [kg/m/s^3]
        dimensionedScalar E_;


public:

    //- Runtime type information
    TypeName("constant");


    // Constructors

        //- Construct from components
        blastConstant(const dictionary& dict, const fvMesh& mesh);


    //- Destructor
    virtual ~blastConstant();


    // Member Functions

        // Access

            //- Absorption coefficient for continuous phase
            virtual tmp<volScalarField> aCont(const label bandI = 0) const;

            //- Absorption coefficient for continuous phase
            virtual scalar cellaCont
            (
                const label celli,
                const label bandI = 0
            ) const;

            //- Emission coefficient for continuous phase
            virtual tmp<volScalarField> eCont(const label bandI = 0) const;

            //- Emission coefficient for continuous phase
            virtual scalar celleCont
            (
                const label celli,
                const label bandI = 0
            ) const;

            //- Emission contribution for continuous phase
            virtual tmp<volScalarField> ECont(const label bandI = 0) const;

            //- Emission contribution for continuous phase
            virtual scalar cellECont
            (
                const label celli,
                const label bandI = 0
            ) const;


    // Member Functions

        inline bool isGrey() const
        {
            return true;
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace absorptionEmissionModels
} // End namespace radiationModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
