/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
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
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "coupledGlobalPolyPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::pointInterpolate
(
    const Field<Type>& pField
) const
{
    if (pField.size() == patch().localPoints().size())
    {
        return samplePatch().globalPointToPatch
        (
            this->patchToPatchInterpolator().transferPoints
            (
                globalPatch(),
                this->patchPointToGlobal(pField)
            )
        );
    }
    else
    {
        FatalErrorInFunction
            << "Patch field does not correspond to patch points." << nl
            << "Patch size: " << patch().localPoints().size() << nl
            << "Field size: " << pField.size()
            << abort(FatalError);
        return tmp<Field<Type>>();
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::pointInterpolate
(
    const tmp<Field<Type>>& pField
) const
{
    return pointInterpolate(pField());
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::faceInterpolate
(
    const Field<Type>& fField
) const
{
    if (fField.size() == patch().size())
    {
        return samplePatch().globalFaceToPatch
        (
            this->patchToPatchInterpolator().transferFaces
            (
                globalPatch(),
                this->patchFaceToGlobal(fField)
            )
        );
    }
    else
    {
        FatalErrorInFunction
            << "Patch field does not correspond to patch points." << nl
            << "Patch size: " << patch().size() << nl
            << "Field size: " << fField.size()
            << abort(FatalError);
        return tmp<Field<Type>>();
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::faceInterpolate
(
    const tmp<Field<Type>>& fField
) const
{

    return faceInterpolate(fField());
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::faceToPointInterpolate
(
    const Field<Type>& fField
) const
{
    if (fField.size() == patch().size())
    {
        const globalPolyPatch& sPatch(samplePatch());
        return sPatch.globalPointToPatch
        (
            patchToPatchInterpolator().transferPoints
            (
                globalPatch(),
                interpolator().faceToPointInterpolate
                (
                    this->patchFaceToGlobal(fField)
                )
            )
        );
    }
    else
    {
        FatalErrorInFunction
            << "Patch field does not correspond to patch points." << nl
            << "Patch size: " << patch().size() << nl
            << "Field size: " << fField.size()
            << abort(FatalError);
        return tmp<Field<Type>>();
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::faceToPointInterpolate
(
    const tmp<Field<Type>>& fField
) const
{

    return faceToPointInterpolate(fField());
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::pointToFaceInterpolate
(
    const Field<Type>& pField
) const
{

    if (pField.size() == samplePatch().patch().localPoints().size())
    {
        const globalPolyPatch& sPatch(samplePatch());
        return sPatch.globalFaceToPatch
        (
            this->patchToPatchInterpolator().transferFaces
            (
                globalPatch(),
                this->interpolator().pointToFaceInterpolate
                (
                    samplePatch().patchPointToGlobal(pField)
                )
            )
        );
    }
    else
    {
        FatalErrorInFunction
            << "Patch field does not correspond to patch points." << nl
            << "Patch size: " << samplePatch().patch().localPoints().size() << nl
            << "Field size: " << pField.size()
            << abort(FatalError);
        return tmp<Field<Type>>();
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledGlobalPolyPatch::pointToFaceInterpolate
(
    const tmp<Field<Type>>& pField
) const
{

    return pointToFaceInterpolate(pField());
}


template<class Type>
void Foam::coupledGlobalPolyPatch::setUnmappedFace
(
    Field<Type>& ff,
    const Type& unmapped
) const
{
    UIndirectList<Type>(ff, unmappedFaces_) = unmapped;
}


template<class Type>
void Foam::coupledGlobalPolyPatch::setUnmappedFace
(
    Field<Type>& ff,
    const Field<Type>& unmapped
) const
{
    UIndirectList<Type>(ff, unmappedFaces_) =
        UIndirectList<Type>(unmapped, unmappedFaces_);
}


template<class Type>
void Foam::coupledGlobalPolyPatch::setUnmappedFace
(
    Field<Type>& ff,
    const tmp<Field<Type>>& tunmapped
) const
{
    setUnmappedFace(ff, tunmapped());
}


template<class Type>
void Foam::coupledGlobalPolyPatch::setUnmappedPoint
(
    Field<Type>& pf,
    const Type& unmapped
) const
{
    UIndirectList<Type>(pf, unmappedPoints_) = unmapped;
}


template<class Type>
void Foam::coupledGlobalPolyPatch::setUnmappedPoint
(
    Field<Type>& pf,
    const Field<Type>& unmapped
) const
{
    UIndirectList<Type>(pf, unmappedPoints_) =
        UIndirectList<Type>(unmapped, unmappedPoints_);
}


template<class Type>
void Foam::coupledGlobalPolyPatch::setUnmappedPoint
(
    Field<Type>& pf,
    const tmp<Field<Type>>& tunmapped
) const
{
    setUnmappedFace(pf, tunmapped());
}


// ************************************************************************* //
