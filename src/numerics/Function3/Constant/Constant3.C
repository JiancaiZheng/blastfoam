/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
03-12-2021 Synthetik Applied Technologies : Added Function3
-------------------------------------------------------------------------------
License
    This file is a derived work of OpenFOAM.

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

#include "Constant3.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::Function3s::Constant<Type>::Constant
(
    const word& name,
    const Type& val
)
:
    FieldFunction3<Type, Constant<Type>>(name),
    value_(val)
{}


template<class Type>
Foam::Function3s::Constant<Type>::Constant
(
    const word& name,
    const dictionary& dict
)
:
    FieldFunction3<Type, Constant<Type>>(name),
    value_(Zero)
{
    if (!dict.found(name))
    {
        dict.lookup("value") >> value_;
    }
    else
    {
        Istream& is(dict.lookup(name));
        word entryType(is);
        if (is.eof())
        {
            dict.lookup("value") >> value_;
        }
        else
        {
            is  >> value_;
        }
    }
}


template<class Type>
Foam::Function3s::Constant<Type>::Constant
(
    const word& name,
    Istream& is
)
:
    FieldFunction3<Type, Constant<Type>>(name),
    value_(pTraits<Type>(is))
{}


template<class Type>
Foam::Function3s::Constant<Type>::Constant(const Constant<Type>& cnst)
:
    FieldFunction3<Type, Constant<Type>>(cnst),
    value_(cnst.value_)
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class Type>
Foam::Function3s::Constant<Type>::~Constant()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::Function3s::Constant<Type>::write(Ostream& os) const
{
    writeEntry(os, "value", value_);
}


// ************************************************************************* i/
