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

\*---------------------------------------------------------------------------*/

#include "fluidBlastThermo.H"
#include "basicFluidBlastThermo.H"
#include "eBlastThermo.H"
#include "forBlastGases.H"
#include "forBlastLiquids.H"
#include "forBlastSolidFluids.H"
#include "makeBlastThermo.H"
#include "addToRunTimeSelectionTable.H"
#include "tabulatedThermoEOS.H"

#include "tabulatedTransport.H"
#include "icoTabulatedTransport.H"

namespace Foam
{
    forGases
    (
        makeThermo,
        fluidBlastThermo,
        basicFluidBlastThermo,
        eBlastThermo
    );

    forLiquids
    (
        makeThermo,
        fluidBlastThermo,
        basicFluidBlastThermo,
        eBlastThermo
    );

    typedef constTransport<tabulatedThermoEOS<specieBlast>>
        constTransporttabulatedtabulatedspecieBlast;
    makeThermo
    (
        fluidBlastThermo,
        basicFluidBlastThermo,
        eBlastThermo,
        constTransporttabulatedtabulatedspecieBlast
    );

    typedef icoTabulatedTransport<tabulatedThermoEOS<specieBlast>>
        icoTabulatedTransporttabulatedtabulatedspecieBlast;
    makeThermo
    (
        fluidBlastThermo,
        basicFluidBlastThermo,
        eBlastThermo,
        icoTabulatedTransporttabulatedtabulatedspecieBlast
    );

    typedef tabulatedTransport<tabulatedThermoEOS<specieBlast>>
        tabulatedTransporttabulatedtabulatedspecieBlast;
    makeThermo
    (
        fluidBlastThermo,
        basicFluidBlastThermo,
        eBlastThermo,
        tabulatedTransporttabulatedtabulatedspecieBlast
    );
}
// ************************************************************************* //
