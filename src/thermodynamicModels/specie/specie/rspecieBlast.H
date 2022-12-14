/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
2020-04-02 Jeff Heylmun:    Renamed class
-------------------------------------------------------------------------------
License
    This file is derivative work of OpenFOAM.

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
    Foam::rspecieBlast

Description

SourceFiles
    rspecieBlast.C

\*---------------------------------------------------------------------------*/

#ifndef rspecieBlast_H
#define rspecieBlast_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "word.H"
#include "scalar.H"
#include "dictionary.H"

#include "thermodynamicConstants.H"
using namespace Foam::constant::thermodynamic;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class rspecieBlast;

inline rspecieBlast operator+(const rspecieBlast&, const rspecieBlast&);
inline rspecieBlast operator*(const scalar, const rspecieBlast&);
inline rspecieBlast operator==(const rspecieBlast&, const rspecieBlast&);

Ostream& operator<<(Ostream&, const rspecieBlast&);

/*---------------------------------------------------------------------------*\
                           Class rspecieBlast Declaration
\*---------------------------------------------------------------------------*/

class rspecieBlast
{
protected:

    // Protected data

        //- Name of specie
        word specieName_;

        //- Number of moles of this component in the mixture
        scalar Y_;

        //- Molecular weight
        scalar molWeight_;

        //- Is the Cp/Cv model internal energy based
        bool eBased_;


public:

    //- Runtime type information
    ClassName("specie");


    // Constructors

        //- Construct from components without name
        inline rspecieBlast(const scalar Y, const scalar molWeight);

        //- Construct from components with name
        inline rspecieBlast
        (
            const word& name,
            const scalar Y,
            const scalar molWeight
        );

        //- Construct as copy
        inline rspecieBlast(const rspecieBlast&);

        //- Construct as named copy
        inline rspecieBlast(const word& name, const rspecieBlast&);

        //- Construct from dictionary
        rspecieBlast(const dictionary& dict);


    // Member Functions

        // Access

            //- Name
            inline const word& name() const;

            //- Molecular weight [kg/kmol]
            inline scalar W() const;

            //- No of moles of this species in mixture
            inline scalar Y() const;

            //- Gas constant [J/(kg K)]
            inline scalar R() const;

        // IO

            //- Write to Ostream
            void write(Ostream& os) const;


    // Member operators

        inline void operator=(const rspecieBlast&);
        inline void operator+=(const rspecieBlast&);
        inline void operator*=(const scalar);


    // Friend operators

        inline friend rspecieBlast operator+
        (
            const rspecieBlast&,
            const rspecieBlast&
        );
        inline friend rspecieBlast operator*(const scalar, const rspecieBlast&);
        inline friend rspecieBlast operator==
        (
            const rspecieBlast&,
            const rspecieBlast&
        );


    // Ostream Operator

        friend Ostream& operator<<(Ostream&, const rspecieBlast&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "rspecieBlastI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
