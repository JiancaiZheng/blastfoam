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

#include "Constant.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
Foam::autoPtr<Foam::Function3<Type>> Foam::Function3<Type>::New
(
    const word& name,
    const dictionary& dict
)
{
    if (dict.isDict(name))
    {
        const dictionary& coeffsDict(dict.subDict(name));

        const word Function3Type(coeffsDict.lookup("type"));

        typename dictionaryConstructorTable::iterator cstrIter =
            dictionaryConstructorTablePtr_->find(Function3Type);

        if (cstrIter == dictionaryConstructorTablePtr_->end())
        {
            FatalErrorInFunction
                << "Unknown Function3 type "
                << Function3Type << " for Function3 "
                << name << nl << nl
                << "Valid Function3 types are:" << nl
                << dictionaryConstructorTablePtr_->sortedToc() << nl
                << exit(FatalError);
        }

        return cstrIter()(name, coeffsDict);
    }
    else
    {
        Istream& is(dict.lookup(name, false));

        token firstToken(is);
        word Function3Type;

        if (!firstToken.isWord())
        {
            is.putBack(firstToken);
            return autoPtr<Function3<Type>>
            (
                new Function3s::Constant<Type>(name, is)
            );
        }
        else
        {
            Function3Type = firstToken.wordToken();
        }

        typename dictionaryConstructorTable::iterator cstrIter =
            dictionaryConstructorTablePtr_->find(Function3Type);

        if (cstrIter == dictionaryConstructorTablePtr_->end())
        {
            FatalErrorInFunction
                << "Unknown Function3 type "
                << Function3Type << " for Function3 "
                << name << nl << nl
                << "Valid Function3 types are:" << nl
                << dictionaryConstructorTablePtr_->sortedToc() << nl
                << exit(FatalError);
        }

        return cstrIter()(name, dict);
    }
}


// ************************************************************************* //
