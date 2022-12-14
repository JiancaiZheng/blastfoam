/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2022
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

Class
    Foam::vtkTimeSeries

Description
    Class to handle writing a vtk time series

SourceFiles
    vtkTimeSeries.C

\*---------------------------------------------------------------------------*/

#ifndef vtkTimeSeries_H
#define vtkTimeSeries_H

#include "HashSet.H"
#include "regIOobject.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

typedef HashSet<scalar, Hash<scalar>> scalarHashSet;

/*---------------------------------------------------------------------------*\
        Class vtkTimeSeries Declaration
\*---------------------------------------------------------------------------*/

class vtkTimeSeries
:
    public scalarHashSet
{
    // Directory the *.vtk.series is written to
    fileName outputDir_;

public:

    //- Runtime type information
    TypeName("vtkTimeSeries");


    // Constructors

        //- Construct and read
        vtkTimeSeries
        (
            const fileName& path,
            const bool nRemove,
            const bool read
        );


    //- Destructor
    virtual ~vtkTimeSeries();


    // Member Functions

        //- Insert a new entry by parsing the given path
        //  Optionally remove components
        bool insertFromPath(const fileName& path, const label nRemove = 0);

        //- Write the updated time series file
        bool writeTimeSeries
        (
            const fileName& name
        ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
