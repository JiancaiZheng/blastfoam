/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021
     \\/     M anipulation  | Synthetik Applied Technologies
-------------------------------------------------------------------------------
License
    This file is a derivative work of OpenFOAM.

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

#include "topoSetList.H"
#include "topoSetSource.H"
#include "coupledPolyPatch.H"
#include "syncTools.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(topoSetList, 0);

    template<>
    const char* NamedEnum<topoSetList::SelectionType, 5>::names[] =
    {
        "all",
        "internal",
        "interface",
        "boundary",
        "interfaceAndBoundary"
    };

    const NamedEnum<topoSetList::SelectionType, 5>
        topoSetList::selectionNames;
}


// * * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * //

Foam::labelList Foam::topoSetList::extractInterfaceCells
(
    const polyMesh& mesh,
    const labelList& cells
)
{
    labelHashSet cellSet(cells);
    labelList newCells(cells);

    label I = 0;
    forAll(cells, ci)
    {
        const label celli = cells[ci];
        const labelList& cc = mesh.cellCells()[celli];
        bool found = cellSet.found(celli);
        bool allSame = true;
        forAll(cc, cj)
        {
            if (cellSet.found(cc[cj]) != found)
            {
                allSame = false;
                break;
            }
        }
        if (!allSame)
        {
            newCells[I++] = celli;
        }
    }
    newCells.resize(I);
    return newCells;
}


Foam::autoPtr<Foam::topoSet> Foam::topoSetList::extractInterfaceCells
(
    const polyMesh& mesh,
    const topoSet& cells
)
{
    autoPtr<topoSet> selectedCells
    (
        topoSet::New
        (
            cells.type(),
            mesh,
            cells.name(),
            cells
        )
    );
    selectedCells->set
    (
        extractInterfaceCells(mesh, cells.toc())
    );
    selectedCells->sync(mesh);
    return selectedCells;
}


Foam::autoPtr<Foam::topoSet> Foam::topoSetList::extractSelectedFaces
(
    const polyMesh& mesh,
    const dictionary& dict,
    const topoSet& faces,
    const bool defaultAll
)
{
    labelHashSet facesToKeep
    (
        extractSelectedFaces(mesh, dict, faces.toc(), defaultAll)
    );

    autoPtr<topoSet> selectedFaces
    (
        topoSet::New
        (
            faces.type(),
            mesh,
            faces.name(),
            faces
        )
    );
    if (isA<faceZoneSet>(faces))
    {
        const faceZoneSet& origFzs = dynamicCast<const faceZoneSet>(faces);
        faceZoneSet& fzs = dynamicCast<faceZoneSet>(selectedFaces());

        extractSelectedFaces
        (
            mesh,
            dict,
            origFzs.addressing(),
            origFzs.flipMap(),
            fzs.addressing(),
            fzs.flipMap(),
            defaultAll
        );
        fzs.updateSet();
    }
    else
    {

        boolList flipMap;
        labelList addressing = extractSelectedFaces
        (
            mesh,
            dict,
            faces.toc(),
            defaultAll
        );
        static_cast<labelHashSet&>(selectedFaces()) = labelHashSet(addressing);
        selectedFaces->sync(mesh);
    }
    return selectedFaces;
}


Foam::labelList Foam::topoSetList::extractSelectedFaces
(
    const polyMesh& mesh,
    const dictionary& dict,
    const labelList& faces,
    const bool defaultAll
)
{
    labelList newFaces;
    boolList flipMap;
    extractSelectedFaces
    (
        mesh,
        dict,
        faces,
        boolList(faces.size(), false),
        newFaces,
        flipMap,
        defaultAll
    );
    return newFaces;
}


void Foam::topoSetList::extractSelectedFaces
(
    const polyMesh& mesh,
    const dictionary& dict,
    const labelList& faces,
    const boolList& flipMap,
    labelList& newFaces,
    boolList& newFlipMap,
    const bool defaultAll
)
{
    const SelectionType sType =
        defaultAll
      ? selectionNames[dict.lookupOrDefault<word>("selectionMode", "all")]
      : selectionNames[dict.lookup<word>("selectionMode")];

    // Return if the selection is empty of all faces are selected
    if (!returnReduce(faces.size(), sumOp<label>()))
    {
        newFaces.clear();
        newFlipMap.clear();
        return;
    }
    if (sType == ALL)
    {
        newFaces = faces;
        newFlipMap = flipMap;
        return;
    }

    // Create an easily searchable list of selected faces for searching
    Map<bool> flipMapMap(faces.size());
    forAll(faces, fi)
    {
        flipMapMap.insert(faces[fi], flipMap[fi]);
    }

    Map<bool> selectedFaces(mesh.nFaces());

    // Add internal faces (coupled faces are included)
    if (sType == INTERNAL)
    {
        forAllConstIter(Map<bool>, flipMapMap, iter)
        {
            const label facei = iter.key();
            if (facei >= mesh.nInternalFaces())
            {
                const label patchi = mesh.boundaryMesh().whichPatch(facei);
                if (isA<coupledPolyPatch>(mesh.boundaryMesh()[patchi]))
                {
                    selectedFaces.insert(iter.key(), iter());
                }
            }
            else
            {
                selectedFaces.insert(iter.key(), iter());
            }
        }
    }

    // Add boundary faces on the selected faces
    if (sType == BOUNDARY || sType == INTERFACE_AND_BOUNDARY)
    {
        wordReList patchNames(dict.lookup("patches"));

        forAll(patchNames, i)
        {
            const label patchi =
                mesh.boundaryMesh().findIndex(patchNames[i]);
            const polyPatch& pp = mesh.boundaryMesh()[patchi];

            forAll(pp, fi)
            {
                const label facei = pp.start() + fi;
                if (flipMapMap.found(facei))
                {
                    selectedFaces.insert(facei, flipMapMap[facei]);
                }
            }
        }
    }

    // Select interface faces, i.e. selected faces whos owner/neighbour
    // cells have some faces that are not selected
    if (sType == INTERFACE || sType == INTERFACE_AND_BOUNDARY)
    {
        const labelList& owner = mesh.faceOwner();
        const labelList& neighbour = mesh.faceNeighbour();
        PackedBoolList markedFaces(mesh.nFaces());
        forAllConstIter(Map<bool>, flipMapMap, iter)
        {
            const label facei = iter.key();
            bool added = false;
            {
                const cell& c = mesh.cells()[owner[facei]];
                forAll(c, fj)
                {
                    if (!flipMapMap.found(c[fj]))
                    {
                        markedFaces.set(facei);
                        added = true;
                        break;
                    }
                }
            }
            if (facei < mesh.nInternalFaces() && !added)
            {
                const cell& c = mesh.cells()[neighbour[facei]];
                forAll(c, fj)
                {
                    if (!flipMapMap.found(c[fj]))
                    {
                        markedFaces.set(facei);
                        break;
                    }
                }
            }
        }

        syncTools::syncFaceList(mesh, markedFaces, orEqOp<uint>());

        labelList used(markedFaces.used());
        forAll(used, fi)
        {
            selectedFaces.insert(used[fi], true);
        }
    }

    newFaces.resize(selectedFaces.size(), -1);
    newFlipMap.resize(selectedFaces.size(), false);
    label fi = 0;
    forAllConstIter(Map<bool>, selectedFaces, iter)
    {
        newFaces[fi] = iter.key();
        newFlipMap[fi] = iter();
        fi++;
    }
}


Foam::labelList Foam::topoSetList::extractSelectedPoints
(
    const polyMesh& mesh,
    const dictionary& dict,
    const labelList& points,
    const bool defaultAll
)
{
    const SelectionType sType =
        defaultAll
      ? selectionNames[dict.lookupOrDefault<word>("selectionMode", "all")]
      : selectionNames[dict.lookup("selectionMode")];

    // Return if the selection is empty of all points are selected
    if (!returnReduce(points.size(), sumOp<label>()) || sType == ALL)
    {
        return points;
    }

    // Create a searchable list of selected points
    const labelHashSet selectedPoints(points);

    // Select all boundary points
    labelHashSet boundaryPoints;
    if (sType == BOUNDARY || sType == INTERFACE_AND_BOUNDARY)
    {
        // Only select points on the given patches
        labelHashSet patchIDs
        (
            mesh.boundaryMesh().patchSet
            (
                dict.lookup<wordReList>("patches")
            )
        );

        // Loop through all the selected patches and add the selected points
        forAll(patchIDs, i)
        {
            const label patchi = patchIDs[i];
            const labelList& ppoints
            (
                mesh.boundaryMesh()[patchi].meshPoints()
            );
            forAll(ppoints, pi)
            {
                if (selectedPoints.found(ppoints[pi]))
                {
                    boundaryPoints.insert(ppoints[pi]);
                }
            }
        }

        if (sType == BOUNDARY)
        {
            return boundaryPoints.toc();
        }
    }

    // The internal points are the entire selection minus the boundary points
    if (sType == INTERNAL)
    {
        labelHashSet internalPoints(points);
        internalPoints -= boundaryPoints;
        return internalPoints.toc();
    }


    // Select the interface points, i.e. selected points for neighbour
    // points that are not selected
    const labelListList& pointPoints = mesh.pointPoints();
    labelHashSet interfacePoints;
    forAll(points, pi)
    {
        const label pointi = points[pi];
        const labelList& pp = pointPoints[pointi];

        bool add = false;
        forAll(pp, pj)
        {
            if (!selectedPoints.found(pp[pj]))
            {
                add = true;
                break;
            }
        }
        if (add)
        {
            interfacePoints.insert(pointi);
        }
    }

    if (sType == INTERFACE)
    {
        return interfacePoints.toc();
    }
    else if (sType == INTERFACE_AND_BOUNDARY)
    {
        return (boundaryPoints | interfacePoints).toc();
    }
    else
    {
        FatalErrorInFunction
            << "Unknown selection mode " << dict.lookup<word>("selectionMode")
            << ", valid selection modes are: " << nl
            << selectionNames.toc() << endl
            << abort(FatalError);
    }
    return points;
}


Foam::autoPtr<Foam::topoSet> Foam::topoSetList::extractSelectedPoints
(
    const polyMesh& mesh,
    const dictionary& dict,
    const topoSet& points,
    const bool defaultAll
)
{
    autoPtr<topoSet> selectedPoints
    (
        topoSet::New
        (
            points.type(),
            mesh,
            points.name(),
            points
        )
    );
    selectedPoints->set
    (
        extractSelectedPoints(mesh, dict, points.toc(), defaultAll)
    );
    selectedPoints->sync(mesh);
    return selectedPoints;
}


// * * * * * * * * * * * * * * * * Constructor * * * * * * * * * * * * * * * //

Foam::topoSetList::topoSetList(const polyMesh& mesh)
:
    TopoSetList
    (
        mesh,
        IOobject
        (
            typeName,
            mesh.facesInstance(),
            mesh
        )
    )
{
    wordList cellZoneNames = mesh.cellZones().names();
    forAll(mesh.cellZones(), czi)
    {
        const cellZone& cz = mesh.cellZones()[czi];
        cellZoneSet* czs = new cellZoneSet
        (
            mesh_,
            cz.name(),
            cz.size()
        );
        (*czs).addressing() = cz;
        czs->updateSet();

        cellTopoSets_.insert(cellZoneNames[czi], czs);
        cellZones_.insert(cellZoneNames[czi]);
    }

    wordList faceZoneNames = mesh.faceZones().names();
    forAll(mesh.faceZones(), fzi)
    {
        const faceZone& fz = mesh.faceZones()[fzi];
        faceZoneSet* fzs = new faceZoneSet
        (
            mesh_,
            fz.name(),
            fz.size()
        );
        (*fzs).addressing() = fz;
        (*fzs).flipMap() = fz.flipMap();
        fzs->updateSet();

        faceTopoSets_.insert(faceZoneNames[fzi], fzs);
        faceZones_.insert(faceZoneNames[fzi]);
    }

    wordList pointZoneNames = mesh.pointZones().names();
    forAll(mesh.pointZones(), pzi)
    {
        const pointZone& pz = mesh.pointZones()[pzi];
        pointZoneSet* pzs = new pointZoneSet
        (
            mesh_,
            pz.name(),
            pz.size()
        );
        (*pzs).addressing() = pz;
        pzs->updateSet();

        pointTopoSets_.insert(pointZoneNames[pzi], pzs);
        pointZones_.insert(pointZoneNames[pzi]);
    }
}


// * * * * * * * * * * * * * * * * Destructor * * * * * * * * * * * * * * * //

Foam::topoSetList::~topoSetList()
{}


// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * * //


void Foam::topoSetList::updateCells
(
    const dictionary& dict,
    const labelList& selectedCells,
    const bool isZone
)
{
    word setDictName = "cell" + word(isZone ? "Zones" : "Sets");
    word setType = "cell" + word(isZone ? "ZoneSet" : "Set");
    if (!dict.found(setDictName))
    {
        return;
    }
    autoPtr<topoSet> selectedCellSet
    (
        topoSet::New
        (
            setType,
            mesh_,
            setType,
            selectedCells.size()
        )
    );
    selectedCellSet().set(selectedCells);
    selectedCellSet().sync(mesh_);

    PtrList<dictionary> setDicts(dict.lookup(setDictName));
    forAll(setDicts, seti)
    {
        modifyTopoSet
        (
            cellTopoSets_,
            cellZones_,
            cellSets_,
            selectedCellSet(),
            setDicts[seti]
        );
    }
}


void Foam::topoSetList::updateFaces
(
    const dictionary& dict,
    const labelList& selectedFaces,
    const boolList& flipMap,
    const bool isZone
)
{
    word setDictName = "face" + word(isZone ? "Zones" : "Sets");
    word setType = "face" + word(isZone ? "ZoneSet" : "Set");
    if (!dict.found(setDictName))
    {
        return;
    }
    autoPtr<topoSet> selectedFaceSet
    (
        topoSet::New
        (
            setType,
            mesh_,
            setType,
            selectedFaces.size()
        )
    );
    if (isZone)
    {
        faceZoneSet& fzs = dynamicCast<faceZoneSet>(selectedFaceSet());
        if (selectedFaces.size() != flipMap.size())
        {
            FatalErrorInFunction
                << "The number of selected faces is not equal to the "
                << "size of the flipMap." << nl
                << "This is probably not using a faceZone source when "
                << "selecting faces." << endl
                << abort(FatalError);
        }
        fzs.addressing() = selectedFaces;
        fzs.flipMap() = flipMap;
        fzs.updateSet();
    }
    else
    {
        selectedFaceSet->set(selectedFaces);
    }
    selectedFaceSet->sync(mesh_);

    PtrList<dictionary> setDicts(dict.lookup(setDictName));
    forAll(setDicts, seti)
    {
        modifyTopoSet
        (
            faceTopoSets_,
            faceZones_,
            faceSets_,
            extractSelectedFaces
            (
                setDicts[seti],
                selectedFaceSet(),
                false
            )(),
            setDicts[seti]
        );
    }
}


void Foam::topoSetList::updatePoints
(
    const dictionary& dict,
    const labelList& selectedPoints,
    const bool isZone
)
{
    word setDictName = "point" + word(isZone ? "Zones" : "Sets");
    word setType = "point" + word(isZone ? "ZoneSet" : "Set");
    if (!dict.found(setDictName))
    {
        return;
    }
    autoPtr<topoSet> selectedPointSet
    (
        topoSet::New
        (
            setType,
            mesh_,
            setType,
            selectedPoints.size()
        )
    );
    selectedPointSet().set(selectedPoints);
    selectedPointSet().sync(mesh_);

    PtrList<dictionary> setDicts(dict.lookup(setDictName));
    forAll(setDicts, seti)
    {
        modifyTopoSet
        (
            pointTopoSets_,
            pointZones_,
            pointSets_,
            extractSelectedPoints
            (
                setDicts[seti],
                selectedPointSet(),
                false
            )(),
            setDicts[seti]
        );
    }
}


void Foam::topoSetList::update
(
    const dictionary& dict,
    const labelList& selectedCells,
    const labelList& selectedFaces,
    const boolList& flipMap,
    const labelList& selectedPoints
)
{
    // Sets
    updateCells(dict, selectedCells, false);
    updateFaces(dict, selectedFaces, flipMap, false);
    updatePoints(dict, selectedPoints, false);

    // Zones
    updateCells(dict, selectedCells, true);
    updateFaces(dict, selectedFaces, flipMap, true);
    updatePoints(dict, selectedPoints, true);
}


void Foam::topoSetList::modifyTopoSet
(
    HashPtrTable<topoSet>& topoSets,
    wordHashSet& zones,
    wordHashSet& sets,
    const topoSet& selectedSet,
    const dictionary& dict
)
{
    const word setName(dict.lookup("name"));
    const word setType = selectedSet.type();
    const bool isZone(label(setType.find("Zone")) >= 0);

    topoSetSource::setAction action = topoSetSource::toAction
    (
        dict.lookup<word>("action")
    );

    if (action == topoSetSource::NEW)
    {
        topoSets.set
        (
            setName,
            topoSet::New(setType, mesh_, setName, selectedSet.size()).ptr()
        );
    }
    else if (action == topoSetSource::CLEAR)
    {
        topoSets.set
        (
            setName,
            topoSet::New(setType, mesh_, setName, 0).ptr()
        );
    }
    else
    {
        if (!topoSets.found(setName))
        {
            if (dict.found("source"))
            {
                const word sourceName(dict.lookup("source"));
                if (!topoSets.found(sourceName))
                {
                    topoSets.set
                    (
                        sourceName,
                        topoSet::New
                        (
                            setType,
                            mesh_,
                            sourceName,
                            IOobject::MUST_READ
                        ).ptr()
                    );
                }
                topoSets.set
                (
                    setName,
                    topoSet::New
                    (
                        setType,
                        mesh_,
                        setName,
                        *topoSets[sourceName]
                    ).ptr()
                );
            }
            else
            {
                Info<< "Source was not specified so reading " << setName
                    << endl;
                topoSets.set
                (
                    setName,
                    topoSet::New
                    (
                        setType,
                        mesh_,
                        setName,
                        IOobject::MUST_READ
                    ).ptr()
                );
                sets.insert(setName);
            }
        }
    }
    topoSet& currentSet = *topoSets[setName];

    // Handle special actions (clear, invert) locally, rest through
    // sources.
    switch (action)
    {
        case topoSetSource::NEW:
        case topoSetSource::ADD:
        {
            currentSet.addSet(selectedSet);

            if (isZone)
            {
                zones.insert(setName);
            }
            else
            {
                sets.insert(setName);
            }
        }
        break;

        case topoSetSource::DELETE:
        {
            HashPtrTable<topoSet>::iterator iter = topoSets.find(setName);
            topoSets.erase(iter);
        }
        break;

        case topoSetSource::SUBSET:
        {
            // Backup current set.
            autoPtr<topoSet> oldSet
            (
                topoSet::New
                (
                    setType,
                    mesh_,
                    currentSet.name() + "_old2",
                    selectedSet.size()
                )
            );
            oldSet->addSet(selectedSet);

            const word sourceName(dict.lookup("source"));
            const topoSet* sourcePtr = nullptr;
            if (!topoSets.found(sourceName))
            {
                sourcePtr = topoSet::New
                (
                    setType,
                    mesh_,
                    sourceName,
                    IOobject::MUST_READ
                ).ptr();
            }
            else
            {
                sourcePtr = topoSets[sourceName];
            }
            currentSet = *sourcePtr;

            if (!topoSets.found(sourceName))
            {
                deleteDemandDrivenData(sourcePtr);
            }

            // Combine new value of currentSet with old one.
            currentSet.subset(oldSet);

            if (isZone)
            {
                zones.insert(setName);
            }
            else
            {
                sets.insert(setName);
            }
        }
        break;

        case topoSetSource::CLEAR:
        {
            break;
        }

        case topoSetSource::INVERT:
        {
            currentSet.invert(currentSet.maxSize(mesh_));

            if (isZone)
            {
                zones.insert(setName);
            }
            else
            {
                sets.insert(setName);
            }
            break;
        }

        case topoSetSource::REMOVE:
        {
            currentSet.deleteSet(selectedSet);

            if (isZone)
            {
                zones.insert(setName);
            }
            else
            {
                sets.insert(setName);
            }
        }
        break;


        default:
            WarningInFunction
                << "Unhandled action " << action << endl;
        break;
    }

    // Synchronise for coupled patches.
    currentSet.sync(mesh_);
}


Foam::labelList Foam::topoSetList::extractInterfaceCells
(
    const labelList& cells
) const
{
    return extractInterfaceCells(mesh_, cells);
}


Foam::autoPtr<Foam::topoSet> Foam::topoSetList::extractInterfaceCells
(
    const topoSet& cells
) const
{
    return extractInterfaceCells(mesh_, cells);
}


//- Remove faces without an owner and a neighbour
Foam::autoPtr<Foam::topoSet> Foam::topoSetList::extractSelectedFaces
(
    const dictionary& dict,
    const topoSet& faces,
    const bool defaultAll
) const
{
    return extractSelectedFaces(mesh_, dict, faces, defaultAll);
}


//- Remove faces without an owner and a neighbour
void Foam::topoSetList::extractSelectedFaces
(
    const dictionary& dict,
    const labelList& faces,
    const boolList& flipMap,
    labelList& newFaces,
    boolList& newFlipMap,
    const bool defaultAll
) const
{
    extractSelectedFaces
    (
        mesh_,
        dict,
        faces,
        flipMap,
        newFaces,
        newFlipMap,
        defaultAll
    );
}


//- Remove faces without an owner and a neighbour
Foam::labelList Foam::topoSetList::extractSelectedFaces
(
    const dictionary& dict,
    const labelList& faces,
    const bool defaultAll
) const
{
    return extractSelectedFaces
    (
        mesh_,
        dict,
        faces,
        defaultAll
    );
}


Foam::labelList Foam::topoSetList::extractSelectedPoints
(
    const dictionary& dict,
    const labelList& points,
    const bool defaultAll
) const
{
    return extractSelectedPoints(mesh_, dict, points, defaultAll);
}


Foam::autoPtr<Foam::topoSet> Foam::topoSetList::extractSelectedPoints
(
    const dictionary& dict,
    const topoSet& points,
    const bool defaultAll
) const
{
    return extractSelectedPoints(mesh_, dict, points, defaultAll);
}


void Foam::topoSetList::clear()
{
    cellTopoSets_.clear();
    faceTopoSets_.clear();
    pointTopoSets_.clear();

    cellSets_.clear();
    cellZones_.clear();

    faceSets_.clear();
    faceZones_.clear();

    pointSets_.clear();
    faceZones_.clear();
}



void Foam::topoSetList::updateMesh(const mapPolyMesh& morphMap)
{
    forAllConstIter(HashPtrTable<topoSet>, cellTopoSets_, iter)
    {
        iter()->updateMesh(morphMap);
    }
    forAllConstIter(HashPtrTable<topoSet>, faceTopoSets_, iter)
    {
        iter()->updateMesh(morphMap);
    }
    forAllConstIter(HashPtrTable<topoSet>, pointTopoSets_, iter)
    {
        iter()->updateMesh(morphMap);
    }
}


void Foam::topoSetList::reorderPatches
(
    const labelUList& newToOld,
    const bool validBoundary
)
{}


void Foam::topoSetList::distribute(const mapDistributePolyMesh& map)
{
    forAllConstIter(HashPtrTable<topoSet>, cellTopoSets_, iter)
    {
        if (isA<cellZoneSet>(*iter()))
        {
            cellZoneSet& czs = dynamicCast<cellZoneSet>(*iter());
            map.distributeCellIndices(czs.addressing());
            czs.updateSet();
        }
        else
        {
            labelList addr(iter()->toc());
            map.distributeFaceIndices(addr);
            static_cast<labelHashSet&>(*iter()) = addr;
            iter()->sync(mesh_);
        }
    }
    forAllConstIter(HashPtrTable<topoSet>, faceTopoSets_, iter)
    {
        if (isA<faceZoneSet>(*iter()))
        {
            faceZoneSet& fzs = dynamicCast<faceZoneSet>(*iter());
            boolList flipMap(mesh_.nFaces(), false);
            forAll(fzs.addressing(), fi)
            {
                flipMap[fzs.addressing()[fi]] = fzs.flipMap()[fi];
            }
            map.distributeFaceIndices(fzs.addressing());
            map.distributeFaceData(flipMap);

            fzs.flipMap().resize(fzs.addressing().size());
            forAll(fzs.addressing(), fi)
            {
                fzs.flipMap()[fi] = flipMap[fzs.addressing()[fi]];
            }
            fzs.updateSet();
        }
        else
        {
            labelList addr(iter()->toc());
            map.distributeFaceIndices(addr);
            static_cast<labelHashSet&>(*iter()) = addr;
            iter()->sync(mesh_);
        }
    }
    forAllConstIter(HashPtrTable<topoSet>, pointTopoSets_, iter)
    {
        if (isA<pointZoneSet>(*iter()))
        {
            pointZoneSet& pzs = dynamicCast<pointZoneSet>(*iter());
            map.distributePointIndices(pzs.addressing());
            pzs.updateSet();
        }
        else
        {
            labelList addr(iter()->toc());
            map.distributePointIndices(addr);
            static_cast<labelHashSet&>(*iter()) = addr;
            iter()->sync(mesh_);
        }
    }
}

void Foam::topoSetList::transferZones(const bool remove)
{
    List<cellZone*> meshCellZones(cellZones_.size());
    List<faceZone*> meshFaceZones(faceZones_.size());
    List<pointZone*> meshPointZones(pointZones_.size());

    wordList cellZones(cellZones_.toc());
    forAll(cellZones, zonei)
    {
        const word& zoneName = cellZones[zonei];
        HashPtrTable<topoSet>::iterator czIter = cellTopoSets_.find(zoneName);
        if (czIter != cellTopoSets_.end())
        {
            meshCellZones[zonei] =
                new cellZone
                (
                    czIter()->name(),
                    czIter()->toc(),
                    zonei,
                    mesh_.cellZones()
                );
            if (remove)
            {
                cellTopoSets_.erase(czIter);
            }

        }
    }

    wordList faceZones(faceZones_.toc());
    forAll(faceZones, zonei)
    {
        const word& zoneName = faceZones[zonei];
        HashPtrTable<topoSet>::iterator fzIter = faceTopoSets_.find(zoneName);
        if (fzIter != faceTopoSets_.end())
        {
            meshFaceZones[zonei] =
                new faceZone
                (
                    fzIter()->name(),
                    fzIter()->toc(),
                    dynamicCast<const faceZoneSet>(*fzIter()).flipMap(),
                    zonei,
                    mesh_.faceZones()
                );
            if (remove)
            {
                faceTopoSets_.erase(fzIter);
            }

        }
    }

    wordList pointZones(pointZones_.toc());
    forAll(pointZones, zonei)
    {
        const word& zoneName = pointZones[zonei];
        HashPtrTable<topoSet>::iterator pzIter = pointTopoSets_.find(zoneName);
        if (pzIter != pointTopoSets_.end())
        {
            meshPointZones[zonei] =
                new pointZone
                (
                    pzIter()->name(),
                    pzIter()->toc(),
                    zonei,
                    mesh_.pointZones()
                );
            if (remove)
            {
                pointTopoSets_.erase(pzIter);
            }

        }
    }
    if (remove)
    {
        cellZones_.clear();
        faceZones_.clear();
        pointZones_.clear();
    }


    if (meshCellZones.size() || meshFaceZones.size() || meshPointZones.size())
    {
        if (meshCellZones.size() && debug)
        {
            Info << "Adding cellZones " << cellZones_ << endl;
        }
        if (meshFaceZones.size() && debug)
        {
            Info << "Adding faceZones " << faceZones_ << endl;
        }
        if (meshPointZones.size() && debug)
        {
            Info << "Adding pointZones " << pointZones_ << endl;
        }
        polyMesh& mesh = const_cast<polyMesh&>(mesh_);
        mesh.pointZones().clear();
        mesh.faceZones().clear();
        mesh.cellZones().clear();

        mesh.addZones(meshPointZones, meshFaceZones, meshCellZones);
    }
}


bool Foam::topoSetList::writeSets() const
{
    List<cellZone*> meshCellZones;
    List<faceZone*> meshFaceZones;
    List<pointZone*> meshPointZones;
    label cellZonei = 0;
    forAllConstIter
    (
        HashPtrTable<topoSet>,
        cellTopoSets_,
        iter
    )
    {
        if (cellZones_.found(iter()->name()))
        {
            meshCellZones.append
            (
                new cellZone
                (
                    iter()->name(),
                    iter()->toc(),
                    cellZonei++,
                    mesh_.cellZones()
                )
            );
        }
        else if (cellSets_.found(iter()->name()))
        {
            DebugInfo<< "Writing cell set " << iter()->name() << endl;
            iter()->instance() = mesh_.facesInstance();
            iter()->write();
        }
    }

    label faceZonei = 0;
    forAllConstIter
    (
        HashPtrTable<topoSet>,
        faceTopoSets_,
        iter
    )
    {
        if (faceZones_.found(iter()->name()))
        {
            meshFaceZones.append
            (
                new faceZone
                (
                    iter()->name(),
                    iter()->toc(),
                    boolList(iter()->size(), false),
                    faceZonei++,
                    mesh_.faceZones()
                )
            );
        }
        else if (faceSets_.found(iter()->name()))
        {
            DebugInfo<< "Writing face set " << iter()->name() << endl;
            iter()->instance() = mesh_.facesInstance();
            iter()->write();
        }
    }

    label pointZonei = 0;
    forAllConstIter
    (
        HashPtrTable<topoSet>,
        pointTopoSets_,
        iter
    )
    {
        if (pointZones_.found(iter()->name()))
        {
            meshPointZones.append
            (
                new pointZone
                (
                    iter()->name(),
                    iter()->toc(),
                    pointZonei++,
                    mesh_.pointZones()
                )
            );
        }
        else if (pointSets_.found(iter()->name()))
        {
            DebugInfo<< "Writing point set " << iter()->name() << endl;
            iter()->instance() = mesh_.facesInstance();
            iter()->write();
        }
    }
    if (cellZonei || faceZonei || pointZonei)
    {
        if (cellZonei && debug)
        {
            Info << "Adding cellZones " << cellZones_ << endl;
        }
        if (faceZonei && debug)
        {
            Info << "Adding faceZones " << faceZones_ << endl;
        }
        if (pointZonei && debug)
        {
            Info << "Adding pointZones " << pointZones_ << endl;
        }
        polyMesh& mesh = const_cast<polyMesh&>(mesh_);
        mesh.pointZones().clear();
        mesh.faceZones().clear();
        mesh.cellZones().clear();

        mesh.addZones(meshPointZones, meshFaceZones, meshCellZones);
        return true;
    }
    return false;
}


// ************************************************************************* //
