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
    Foam::radiationModels::blastNoRadiation

Description
    No radiation - does nothing to energy equation source terms
    (returns zeros)

SourceFiles
    blastNoRadiation.C

\*---------------------------------------------------------------------------*/

#ifndef blastNoRadiation_H
#define blastNoRadiation_H

#include "blastRadiationModel.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace radiationModels
{

/*---------------------------------------------------------------------------*\
                         Class blastNoRadiation Declaration
\*---------------------------------------------------------------------------*/

class blastNoRadiation
:
    public blastRadiationModel
{
public:

    //- Runtime type information
    TypeName("none");


    // Constructors

        //- Construct from components
        blastNoRadiation(const volScalarField& T);

        //- Construct from components
        blastNoRadiation(const dictionary& dict, const volScalarField& T);

        //- Disallow default bitwise copy construction
        blastNoRadiation(const blastNoRadiation&) = delete;


    //- Destructor
    virtual ~blastNoRadiation();


    // Member Functions

        // Edit

            //- Main update/correction routine
            virtual void correct();

            //- Solve radiation equation(s)
            virtual void calculate();

            //- Read radiationProperties dictionary
            bool read();

            //- Source term component (for power of T^4)
            virtual tmp<volScalarField> Rp() const;

            //- Source term component (for power of T^4)
            virtual scalar cellRp(const label celli) const;

            //- Source term component (constant)
            virtual tmp<volScalarField::Internal> Ru() const;

            //- Source term component (constant)
            virtual scalar cellRu(const label celli) const;


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const blastNoRadiation&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace radiationModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
