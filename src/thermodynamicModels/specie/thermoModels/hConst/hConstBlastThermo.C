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

#include "hConstBlastThermo.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class EquationOfState>
Foam::hConstThermo<EquationOfState>::hConstThermo(const dictionary& dict)
:
    EquationOfState(dict),
    Cp_(dict.subDict("thermodynamics").lookup<scalar>("Cp")),
    Hf_(dict.subDict("thermodynamics").lookup<scalar>("Hf")),
    Tref_(dict.subDict("thermodynamics").lookupOrDefault<scalar>("Tref", Tstd)),
    Hsref_(dict.subDict("thermodynamics").lookupOrDefault<scalar>("Hsref", Cp_*Tref_)),
    flameT_(dict.subDict("thermodynamics").lookupOrDefault("flameT", Hf_/Cp_))
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class EquationOfState>
void Foam::hConstThermo<EquationOfState>::write(Ostream& os) const
{
    EquationOfState::write(os);

    dictionary dict("thermodynamics");
    dict.add("Cp", Cp_);
    dict.add("Hf", Hf_);
    if (Tref_ != Tstd)
    {
        dict.add("Tref", Tref_);
    }
    if (Hsref_ != 0)
    {
        dict.add("Hsref", Hsref_);
    }
    if (flameT_ != 0)
    {
        dict.add("flameT", flameT_);
    }
    os  << indent << dict.dictName() << dict;
}


// * * * * * * * * * * * * * * * Ostream Operator  * * * * * * * * * * * * * //

template<class EquationOfState>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const hConstThermo<EquationOfState>& et
)
{
    et.write(os);
    return os;
}

// ************************************************************************* //
