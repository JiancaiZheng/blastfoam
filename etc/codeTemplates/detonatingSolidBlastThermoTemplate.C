/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) YEAR OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "forBlastThermo.H"
#include "makeBlastThermo.H"

#include "${specie}.H"
#include "r${specie}.H"

#include "thermoModel.H"

// EoS
#include "${uEquationOfState}.H"
#include "${rEquationOfState}.H"

// Thermo
#include "${thermo}BlastThermo.H"

// Transport
#include "${transport}Transport.H"

#include "solidBlastThermo.H"
#include "detonatingSolidBlastThermo.H"
#include "blendedBlastThermo.H"

#include "addToRunTimeSelectionTable.H"


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

extern "C"
{
    // dynamicCode:
    // SHA1 = ${SHA1sum}
    //
    // Unique function name that can be checked if the correct library version
    // has been loaded
    void ${typeName}_${SHA1sum}(bool load)
    {
        if (load)
        {
            // code that can be explicitly executed after loading
        }
        else
        {
            // code that can be explicitly executed before unloading
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    forDetThermo
    (
        ${uTransport}SolidTransport,
        ${rTransport}SolidTransport,
        ${uThermo}Thermo,
        ${rThermo}Thermo,
        ${uEquationOfState},
        ${rEquationOfState},
        ${specie},
        makeDetThermo,
        solidBlastThermo,
        detonatingSolidBlastThermo,
        blendedBlastThermo
    );
}

// ************************************************************************* //
