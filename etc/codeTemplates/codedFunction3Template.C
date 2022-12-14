/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) YEAR OpenFOAM Foundation
     \\/     M anipulation  |
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

#include "codedFunction3Template.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace Function3s
{
    defineTypeNameAndDebug(${typeName}Function3${TemplateType}, 0);
}
    Function3<${TemplateType}>::adddictionaryConstructorToTable<Function3s::
        ${typeName}Function3${TemplateType}>
        ${typeName}Function3${TemplateType}ConstructorToTable_;
}


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


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::Function3s::${typeName}Function3${TemplateType}::
${typeName}Function3${TemplateType}
(
    const word& entryName,
    const dictionary& dict
)
:
    FieldFunction3<${TemplateType}, ${typeName}Function3${TemplateType}>
    (
        entryName
    ),
    topDict_(dict.topDict()),
    dict_(topDict_, dict)
{
    if (${verbose:-false})
    {
        Info<< "Construct ${typeName} sha1: ${SHA1sum} from dictionary\n";
    }
}


Foam::Function3s::${typeName}Function3${TemplateType}::
${typeName}Function3${TemplateType}
(
    const ${typeName}Function3${TemplateType}& f3
)
:
    FieldFunction3<${TemplateType}, ${typeName}Function3${TemplateType}>
    (
        f3
    ),
    dict_(f3.dict_)
{
    if (${verbose:-false})
    {
        Info<< "Construct ${typeName} sha1: ${SHA1sum} as copy\n";
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::Function3s::${typeName}Function3${TemplateType}::
~${typeName}Function3${TemplateType}()
{
    if (${verbose:-false})
    {
        Info<< "Destroy ${typeName} sha1: ${SHA1sum}\n";
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::Function3s::${typeName}Function3${TemplateType}::write
(
    Ostream& os
) const
{
    NotImplemented;
}


// ************************************************************************* i/
