/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021
     \\/     M anipulation  | Synthetik Applied Technologies
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

SourceFiles


\*---------------------------------------------------------------------------*/

#include "UniformFieldSetType.H"
#include "FieldSetTypesFwd.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "pointFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace FieldSetTypes
{

template<class Type>
using UniformVol = Uniform<Type, VolFieldSetType>;

template<class Type>
using UniformSurface = Uniform<Type, SurfaceFieldSetType>;

template<class Type>
using UniformPoint = Uniform<Type, PointFieldSetType>;
}

makeFieldSetTypeType(UniformVol, scalar, VolFieldSetType);
makeFieldSetTypeType(UniformVol, vector, VolFieldSetType);
makeFieldSetTypeType(UniformVol, sphericalTensor, VolFieldSetType);
makeFieldSetTypeType(UniformVol, symmTensor, VolFieldSetType);
makeFieldSetTypeType(UniformVol, tensor, VolFieldSetType);

makeFieldSetTypeType(UniformSurface, scalar, SurfaceFieldSetType);
makeFieldSetTypeType(UniformSurface, vector, SurfaceFieldSetType);
makeFieldSetTypeType(UniformSurface, sphericalTensor, SurfaceFieldSetType);
makeFieldSetTypeType(UniformSurface, symmTensor, SurfaceFieldSetType);
makeFieldSetTypeType(UniformSurface, tensor, SurfaceFieldSetType);

makeFieldSetTypeType(UniformPoint, scalar, PointFieldSetType);
makeFieldSetTypeType(UniformPoint, vector, PointFieldSetType);
makeFieldSetTypeType(UniformPoint, sphericalTensor, PointFieldSetType);
makeFieldSetTypeType(UniformPoint, symmTensor, PointFieldSetType);
makeFieldSetTypeType(UniformPoint, tensor, PointFieldSetType);

}
// ************************************************************************* //

