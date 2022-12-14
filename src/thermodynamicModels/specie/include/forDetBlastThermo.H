/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2019-2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
2021-08-02 Jeff Heylmun:    Added blastFoam thermo classes
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

#ifndef forDetBlastThermo_H
#define forDetBlastThermo_H

#include "typedefBlastThermo.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define forDetThermo(uMu, rMu, uCp, rCp, uEos, rEos, Specie, Macro, Args...) \
                                                                            \
    typedefThermo                                                           \
    (                                                                       \
        uMu,                                                                \
        uCp,                                                                \
        uEos,                                                               \
        Specie                                                              \
    );                                                                      \
    typedefThermo                                                           \
    (                                                                       \
        rMu,                                                                \
        rCp,                                                                \
        rEos,                                                               \
        r##Specie                                                           \
    );                                                                      \
                                                                            \
    Macro                                                                   \
    (                                                                       \
        Args,                                                               \
        uMu##uCp##uEos##Specie,                                             \
        rMu##rCp##rEos##r##Specie                                           \
    );


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif
// ************************************************************************* //
