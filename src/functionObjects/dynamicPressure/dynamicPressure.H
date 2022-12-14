/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2016-2018 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
13-05-2020 Synthetik Applied Technologies: | Calculate dynamic pressure
-------------------------------------------------------------------------------
License
    This file is a derivative work of OpenFOAM.

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
    Foam::functionObjects::dynamicPressure

Description
    Calculates and writes the dynamic pressure. Includes option to save the field
    to the database.

See also
    Foam::functionObjects::fieldExpression

SourceFiles
    dynamicPressure.C

\*---------------------------------------------------------------------------*/

#ifndef functionObjects_dynamicPressure_H
#define functionObjects_dynamicPressure_H

#include "fieldExpression.H"
#include "volFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                          Class MachNo Declaration
\*---------------------------------------------------------------------------*/

class dynamicPressure
:
    public fieldExpression
{
    // Private Member data

        //- Name of density field
        word rhoName_;

        //- Name of velocity field
        word UName_;


    // Protected member functions

        virtual bool calc();


public:

    //- Runtime type information
    TypeName("dynamicPressure");

    // Constructors

        //- Construct for given objectRegistry and dictionary.
        //  Allow the possibility to load fields from files
        dynamicPressure
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict
        );

        //- Disallow default bitwise copy construction
        dynamicPressure(const dynamicPressure&) = delete;


    //- Destructor
    virtual ~dynamicPressure();


    // Member Operators

        //- Disallow default bitwise assignment
        void operator=(const dynamicPressure&) = delete;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
