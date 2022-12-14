/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021-2022
     \\/     M anipulation  | Synthetik Applied Technologies
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
    Foam::Simpson13Integrator

Description
   Simpson 1/3 integration method

SourceFiles
    Simpson13Integrator.C

\*---------------------------------------------------------------------------*/

#ifndef Simpson13Integrator_H
#define Simpson13Integrator_H

#include "Integrator.H"
#include "typeInfo.H"
#include "autoPtr.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                        Class Simpson13Integrator Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class Simpson13Integrator
:
    public Integrator<Type>
{
    typedef Equation<Type> equationType;

        //- Basic integration method
        virtual Type integrate_
        (
            const scalar dx,
            const Type& f0,
            const Type& fm,
            const Type& f1
        ) const;

        //- Solve the root
        virtual Type integrate_
        (
            const Type& Q,
            const scalar x0,
            const scalar x1,
            const Type& f0,
            const Type& fm,
            const Type& f1,
            const scalar tol,
            const label li
        ) const;

public:

    //- Runtime type information
    TypeName("Simpson13");

    // Constructors

        //- Construct for given Equation
        Simpson13Integrator
        (
            const equationType& eqn,
            const dictionary& dict
        );

        //- Construct for given Equation
        Simpson13Integrator
        (
            const equationType& eqn,
            const integrator& inter
        );

        //- Disallow default bitwise copy construction
        Simpson13Integrator(const Simpson13Integrator<Type>&) = delete;


    //- Destructor
    virtual ~Simpson13Integrator()
    {}

    // Member Operators

        //- Integrate the equation
        virtual Type integrate
        (
            const scalar x0,
            const scalar x1,
            const label li
        ) const;

        //- Disallow default bitwise assignment
        void operator=(const Simpson13Integrator<Type>&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#include "Simpson13Integrator.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
