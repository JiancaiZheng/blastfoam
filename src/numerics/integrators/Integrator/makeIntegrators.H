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

\*---------------------------------------------------------------------------*/

#ifndef makeIntegrators_H
#define makeIntegrators_H

#include "Integrator.H"
#include "MidPointIntegrator.H"
#include "TrapezoidalIntegrator.H"
#include "Simpson13Integrator.H"
#include "Simpson38Integrator.H"
#include "BooleIntegrator.H"
#include "GaussianIntegrator.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Define all integrators for a given type
#define makeIntegratorTypes(Type)                                              \
    makeIntegratorType(MidPointIntegrator, Type)                               \
    makeIntegratorType(TrapezoidalIntegrator, Type)                            \
    makeIntegratorType(Simpson13Integrator, Type)                              \
    makeIntegratorType(Simpson38Integrator, Type)                              \
    makeIntegratorType(GaussianIntegrator, Type)                               \
    makeIntegratorType(BooleIntegrator, Type)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Make the base integrator type along with the actual models
#define makeIntegrators(Type)                                                  \
    makeIntegrator(Type)                                                       \
    makeIntegratorTypes(Type);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Macro with a dummy first input for use with some macro based construction
#define makeIntegratorsArg(Dummy, Type)                                        \
    makeIntegrator(Type)                                                       \
    makeIntegratorTypes(Type);

#endif


// ************************************************************************* //
