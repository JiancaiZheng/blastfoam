/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019 Synthetik Applied Technologies
     \\/     M anipulation  |
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
    Foam::BirchMurnaghan3

Description
    Third order BirchMurnaghan equation of state for a solid

    References:
    \verbatim
        Zheng, Z., Zhao, J. (2016).
        Unreacted equation of states of typical energetic material under static
        compression: A review
        Chin. Phys. B, 25 (7), 1-11.
    \endverbatim

SourceFiles
    BirchMurnaghan3.C

\*---------------------------------------------------------------------------*/

#ifndef BirchMurnaghan3Blast_H
#define BirchMurnaghan3Blast_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "autoPtr.H"

namespace Foam
{

// Forward declaration of friend functions and operators

template<class Specie> class BirchMurnaghan3;

template<class Specie>
inline BirchMurnaghan3<Specie> operator+
(
    const BirchMurnaghan3<Specie>&,
    const BirchMurnaghan3<Specie>&
);

template<class Specie>
inline BirchMurnaghan3<Specie> operator*
(
    const scalar,
    const BirchMurnaghan3<Specie>&
);

template<class Specie>
inline BirchMurnaghan3<Specie> operator==
(
    const BirchMurnaghan3<Specie>&,
    const BirchMurnaghan3<Specie>&
);

template<class Specie>
Ostream& operator<<
(
    Ostream&,
    const BirchMurnaghan3<Specie>&
);

/*---------------------------------------------------------------------------*\
                           Class BirchMurnaghan Declaration
\*---------------------------------------------------------------------------*/

template<class Specie>
class BirchMurnaghan3
:
    public Specie
{
// Private data

    //- Reference density
    scalar rho0_;

    //- Reference pressure
    scalar pRef_;

    //- Bulk modulus
    scalar K0_;

    //- Derivative of bulk modulus w.r.t. pressure
    scalar K0Prime_;

    //- Mie Grunseisen coefficient
    scalar Gamma_;


public:

    // Constructors

        //- Construct from components
        inline BirchMurnaghan3
        (
            const Specie& sp,
            const scalar rho0,
            const scalar pRef,
            const scalar K0,
            const scalar K0Prime,
            const scalar Gamma
        );

        //- Construct from dictionary
        BirchMurnaghan3(const dictionary& dict);

        //- Construct as named copy
        inline BirchMurnaghan3
        (
            const word& name,
            const BirchMurnaghan3&
        );

        //- Construct and return a clone
        inline autoPtr<BirchMurnaghan3> clone() const;

        // Selector from dictionary
        inline static autoPtr<BirchMurnaghan3> New
        (
            const dictionary& dict
        );


    // Member Functions

        //- Return the instantiated type name
        static word typeName()
        {
            return
                "BirchMurnaghan3<"
              + word(Specie::typeName_()) + '>';
        }

        //- Is temperature used
        static bool temperatureBased()
        {
            return true;
        }

        //- Is this a solid equation of state
        static bool solid()
        {
            return true;
        }

        //- Return pressure optionally limited pressure
        inline scalar p
        (
            const scalar rho,
            const scalar e,
            const scalar T,
            const bool limit = true
        ) const;

        //- Return unlimited pressure
        inline scalar pRhoT
        (
            const scalar rho,
            const scalar T
        ) const;

        //- Return density given temperature and pressure
        inline scalar rho(const scalar p, const scalar T) const;

        //- Gamma function
        inline scalar Gamma
        (
            const scalar rho,
            const scalar e,
            const scalar T,
            const scalar cv
        ) const;

        //- Speed of sound
        inline scalar cSqr
        (
            const scalar p,
            const scalar rho,
            const scalar e,
            const scalar T,
            const scalar cv
        ) const;

        //- Return derivative of pressure w.r.t density
        inline scalar dpdv
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Return derivative of pressure w.r.t. Temperature
        inline scalar dpdT
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Return derivative of pressure w.r.t. internal energy
        inline scalar dpde
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Internal energy correction [J/kg]
        inline scalar E
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Enthalpy correction [J/kg]
        inline scalar H
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Heat capacity at constant volume [J/kg/K]
        inline scalar Cv
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Heat capacity at constant pressure [J/kg/K]
        inline scalar Cp
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Heat capacity difference [J/kg/K]
        inline scalar CpMCv
        (
            const scalar rho,
            const scalar e,
            const scalar T,
            const scalar CpCv
        ) const;

        //- Entropy of mixture [J/kg/K]
        inline scalar S
        (
            const scalar p,
            const scalar T
        ) const;


        // IO

            //- Write to Ostream
            void write(Ostream& os) const;


    // Member operators

        inline void operator+=(const BirchMurnaghan3&);
        inline void operator*=(const scalar);


    // Friend operators

        friend BirchMurnaghan3 operator+ <Specie>
        (
            const BirchMurnaghan3&,
            const BirchMurnaghan3&
        );

        friend BirchMurnaghan3 operator* <Specie>
        (
            const scalar s,
            const BirchMurnaghan3&
        );

        friend BirchMurnaghan3 operator== <Specie>
        (
            const BirchMurnaghan3&,
            const BirchMurnaghan3&
        );


    // Ostream Operator

        friend Ostream& operator<< <Specie>
        (
            Ostream&,
            const BirchMurnaghan3&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "BirchMurnaghan3I.H"

#ifdef NoRepository
    #include "BirchMurnaghan3.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
