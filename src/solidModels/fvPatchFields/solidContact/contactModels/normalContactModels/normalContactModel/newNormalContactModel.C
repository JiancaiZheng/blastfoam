/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2004-2007 Hrvoje Jasak
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

InClass
    normalContactModel

\*---------------------------------------------------------------------------*/

#include "normalContactModel.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

autoPtr<normalContactModel> normalContactModel::New
(
    const word& name,
    const fvPatch& patch,
    const dictionary& dict,
    const label masterPatchID,
    const label slavePatchID,
    const standAlonePatch& masterFaceZonePatch,
    const standAlonePatch& slaveFaceZonePatch
)
{
    Info<< "    Normal contact model: " << name << endl;

    dictionaryConstructorTable::iterator cstrIter =
        dictionaryConstructorTablePtr_->find(name);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        FatalIOErrorIn
        (
            "normalContactModel::New(\n"
            "    const word& name,\n"
            "    const fvPatch& patch,\n"
            "    const dictionary& dict,\n"
            "    const label masterPatchID,\n"
            "    const label slavePatchID,\n"
            "    const standAlonePatch& masterFaceZonePatch,\n"
            "    const standAlonePatch& slaveFaceZonePatch\n"
            ")",
            dict
        )   << "Unknown normalContactModel type "
            << name << endl << endl
            << "Valid  normalContactModels are : " << endl
            << dictionaryConstructorTablePtr_->toc()
            << exit(FatalIOError);
    }

    return autoPtr<normalContactModel>
    (
        cstrIter()
        (
            name,
            patch,
            dict,
            masterPatchID,
            slavePatchID,
            masterFaceZonePatch,
            slaveFaceZonePatch
        )
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
