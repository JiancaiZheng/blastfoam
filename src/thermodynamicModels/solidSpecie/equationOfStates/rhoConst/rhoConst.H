/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
2020-04-02 Jeff Heylmun:    Modified class for a density based thermodynamic
                            class
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
    Foam::rhoConst

Description
    Constant density equation of state


SourceFiles
    rhoConstEquationOfState.C

\*---------------------------------------------------------------------------*/

#ifndef rhoConstBlast_H
#define rhoConstBlast_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "autoPtr.H"

namespace Foam
{

template<class Specie> class rhoConst;

template<class Specie>
inline rhoConst<Specie> operator+
(
    const rhoConst<Specie>&,
    const rhoConst<Specie>&
);

template<class Specie>
inline rhoConst<Specie> operator*
(
    const scalar,
    const rhoConst<Specie>&
);

template<class Specie>
inline rhoConst<Specie> operator==
(
    const rhoConst<Specie>&,
    const rhoConst<Specie>&
);
template<class Specie> class rhoConst;

template<class Specie>
inline rhoConst<Specie> operator+
(
    const rhoConst<Specie>&,
    const rhoConst<Specie>&
);

template<class Specie>
inline rhoConst<Specie> operator*
(
    const scalar,
    const rhoConst<Specie>&
);

template<class Specie>
inline rhoConst<Specie> operator==
(
    const rhoConst<Specie>&,
    const rhoConst<Specie>&
);
/*---------------------------------------------------------------------------*\
                           Class rhoConst Declaration
\*---------------------------------------------------------------------------*/

template<class Specie>
class rhoConst
:
    public Specie
{
// Private data

    //- Density
    scalar rho0_;

    //- Reference pressure
    scalar pRef_;

public:

    // Constructors

        //- Construct from components
        inline rhoConst(const Specie& sp, const scalar rho, const scalar pRef);

        //- Construct from dictionary
        rhoConst(const dictionary& dict);

        //- Construct as named copy
        inline rhoConst(const word& name, const rhoConst&);

        //- Construct and return a clone
        inline autoPtr<rhoConst> clone() const;


    // Member Functions

        //- Return the instantiated type name
        static word typeName()
        {
            return
                "rhoConst<"
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

        inline scalar rho0() const
        {
            return rho0_;
        }

        //- Return deviation from ideal gas pressure
        inline scalar p
        (
            const scalar rho,
            const scalar e,
            const scalar T,
            const bool limit = true
        ) const;

        //- Return non-ideal speed of sound
        inline scalar cSqr
        (
            const scalar p,
            const scalar rho,
            const scalar e,
            const scalar T,
            const scalar cv
        ) const;

        //- Gamma function
        inline scalar Gamma
        (
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

        //- Return derivative of pressure w.r.t. internalEnergy
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

        //- Heat capacity at constant volume of mixture [J/kg/K]
        inline scalar S
        (
            const scalar p,
            const scalar rho,
            const scalar T
        ) const;


        // Member operators

            inline void operator+=(const rhoConst&);
            inline void operator*=(const scalar);

        // Friend operators

        friend rhoConst operator+ <Specie>
        (
            const rhoConst&,
            const rhoConst&
        );

        friend rhoConst operator* <Specie>
        (
            const scalar s,
            const rhoConst&
        );

        friend rhoConst operator== <Specie>
        (
            const rhoConst&,
            const rhoConst&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "rhoConstI.H"

#ifdef NoRepository
    #include "rhoConst.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
