/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2020-2021 OpenFOAM Foundation
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
    Foam::icoTabulatedTransport

Description
    Transport properties package using uniformly-spaced tabulated data for
    viscosity and thermal conductivity vs pressure and temperature.

Usage
    \table
        Property        | Description
        mu              | Dynamic viscosity vs pressure and temperature table
        kappa           | Thermal conductivity vs pressure and temperature table
    \endtable

    Example of the specification of the transport properties:
    \verbatim
    transport
    {
        muCoeffs
        {
            T ( ... );
            mu ( ... );
        }

        kappaCoeffs
        {
            T ( ... );
            kappa ( ... );
        }
    }
    \endverbatim

    See also tutorials/resources/thermoData/wallBoiling*

SourceFiles
    icoTabulatedTransportI.H
    icoTabulatedTransport.C

See also
    Foam::Function2s::uniformTable

\*---------------------------------------------------------------------------*/

#ifndef icoTabulatedTransport_H
#define icoTabulatedTransport_H

#include "lookupTables1D.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators

template<class Thermo> class icoTabulatedTransport;

template<class Thermo>
Ostream& operator<<(Ostream&, const icoTabulatedTransport<Thermo>&);


/*---------------------------------------------------------------------------*\
                     Class icoTabulatedTransport Declaration
\*---------------------------------------------------------------------------*/

template<class Thermo>
class icoTabulatedTransport
:
    public Thermo
{

    // Private Data

        //- Dynamic viscosity table [Pa.s]
        scalarLookupTable1D mu_;

        //- Thermal conductivity table [W/m/K]
        scalarLookupTable1D kappa_;


    // Private Member Functions

        //- Construct from components
        inline icoTabulatedTransport
        (
            const Thermo& t,
            const scalarLookupTable1D& muPoly,
            const scalarLookupTable1D& kappaPoly
        );


public:

    // Constructors

        //- Construct as named copy
        inline icoTabulatedTransport(const word&, const icoTabulatedTransport&);

        //- Construct from dictionary
        icoTabulatedTransport(const dictionary& dict);

        //- Construct and return a clone
        inline autoPtr<icoTabulatedTransport> clone() const;

        // Selector from dictionary
        inline static autoPtr<icoTabulatedTransport> New(const dictionary& dict);


    // Member Functions

        //- Return the instantiated type name
        static word typeName()
        {
            return "icoTabulated<" + Thermo::typeName() + '>';
        }

        //- Dynamic viscosity [kg/m/s]
        inline scalar mu
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Thermal conductivity [W/m/K]
        inline scalar kappa
        (
            const scalar rho,
            const scalar e,
            const scalar T
        ) const;

        //- Write to Ostream
        void write(Ostream& os) const;


    // Ostream Operator

        friend Ostream& operator<< <Thermo>
        (
            Ostream&,
            const icoTabulatedTransport&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "icoTabulatedTransportI.H"

#ifdef NoRepository
    #include "icoTabulatedTransport.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
