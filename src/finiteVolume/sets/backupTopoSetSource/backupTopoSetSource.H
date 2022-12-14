/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
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

Class
    Foam::backupTopoSetSource

Description
    Runtime selectable topoSetSource with a backup region

SourceFiles
    backupTopoSetSource.C

\*---------------------------------------------------------------------------*/

#ifndef backupTopoSetSource_H
#define backupTopoSetSource_H

#include "topoSetSource.H"
#include "polyMesh.H"
#include "topoSet.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{


/*---------------------------------------------------------------------------*\
                    Class backupTopoSetSource Declaration
\*---------------------------------------------------------------------------*/

class backupTopoSetSource
{
protected:
// Protected data

    //- Reference to mesh
    const polyMesh& mesh_;

    //- Stored dictionary
    dictionary dict_;

    //- Is the backup region allowed to be used
    bool allowBackup_;

    //- Actual topoSetSource
    autoPtr<topoSetSource> source_;

    //- Backup topoSetSource
    autoPtr<topoSetSource> backup_;


    //- Selected cells
    labelList selectedCells_;

    //- Selected faces
    labelList selectedFaces_;

    //- Flip map
    boolList flipMap_;

    //- SelectedPoints
    labelList selectedPoints_;


    // Private member functions

        //- Select faces associated with cells
        labelList cellsToFaces(const labelList& cells) const;

        //- Select points associated with cells
        labelList cellsToPoints(const labelList& cells) const;

        //- Select faces associated with cells
        labelList facesToCells(const labelList& faces) const;

        //- Select points associated with cells
        labelList facesToPoints(const labelList& faces) const;

        //- Select faces associated with cells
        labelList pointsToCells(const labelList& points) const;

        //- Select points associated with cells
        labelList pointsToFaces(const labelList& points) const;


public:

    //- Runtime type information
    TypeName("backupTopoSetSource");

    // Constructors

        //- Copy Constructor
        backupTopoSetSource(const backupTopoSetSource&);

        //- Construct from components
        backupTopoSetSource
        (
            const word& topoSetSourceType,
            const polyMesh& mesh,
            const dictionary& dict
        );

        //- Construct from components
        backupTopoSetSource
        (
            const polyMesh& mesh,
            const dictionary& dict,
            autoPtr<topoSetSource>& source
        );

        //- Construct from components
        backupTopoSetSource
        (
            const polyMesh& mesh,
            const dictionary& dict,
            autoPtr<topoSetSource>& source,
            autoPtr<topoSetSource>& backup
        );


    //- Destructor
    virtual ~backupTopoSetSource();

        //- Class used for the read-construction of
        //  PtrLists of backupTopoSetSource
        class iNew
        {
            const polyMesh& mesh_;

        public:

            iNew(const polyMesh& mesh)
            :
                mesh_(mesh)
            {}

            autoPtr<backupTopoSetSource> operator()(Istream& is) const
            {
                word topoSetSourceType(is);
                dictionary dict(is);
                return autoPtr<backupTopoSetSource>
                (
                    new backupTopoSetSource(topoSetSourceType, mesh_, dict)
                );
            }
        };

    //- Clone
    autoPtr<backupTopoSetSource> clone() const
    {
        return autoPtr<backupTopoSetSource>(new backupTopoSetSource(*this));
    }

    // Member Functions

        //- Return the dictionary dict
        const dictionary& dict() const
        {
            return dict_;
        }

        //- Return the setType
        virtual topoSetSource::sourceType setType() const
        {
            return source_->setType();
        }

        //- Allow backup region to be used
        void allowBackup(const bool allow)
        {
            allowBackup_ = allow;
        }

        //- Return selected cells
        const labelList& selectedCells() const
        {
            return selectedCells_;
        }

        //- Return selected faces
        const labelList& selectedFaces() const
        {
            return selectedFaces_;
        }

        //- Return flip map
        const boolList& flipMap() const
        {
            return flipMap_;
        }

        //- Return selected points
        const labelList& selectedPoints() const
        {
            return selectedPoints_;
        }

        //- Return is the source is a cell based source
        bool isCell() const;

        //- Return is the source is a face based source
        bool isFace() const;

        //- Return is the source is a point based source
        bool isPoint() const;

        //- Apply to the given set
        virtual void applyToSet
        (
            const topoSetSource::setAction action,
            topoSet&
        ) const;

        //- Get selections from a given set, based on what setType this is
        void updateSets();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
