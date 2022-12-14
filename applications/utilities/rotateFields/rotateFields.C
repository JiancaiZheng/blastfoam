/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021-2022
     \\/     M anipulation  | SYnthetik Applied Technologies
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

Application
    rotateFields

Description
    Rotate fields from 1-D to 2-D or 2-D to 3-D. Only for
    axisymmetric cases. Currently mapping is not conservative so only the
    nearest cell value (based on cell centres) will be used. Optionally,
    refinement can be used.

\*---------------------------------------------------------------------------*/

#include "fvCFD.H"
#include "labelVector.H"
#include "wedgeFvPatch.H"
#include "IOobjectList.H"
#include "HashSet.H"
#include "UautoPtr.H"
#include "genericFvPatchField.H"
#include "indexedOctree.H"
#include "treeDataCell.H"

#include "fvMeshRefiner.H"
#include "errorEstimator.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool parRun = false;
void setParRun(const bool par)
{
    Pstream::parRun() = par;
}
void resetParRun()
{
    Pstream::parRun() = parRun;
}

struct cellInfo
{
    label proc;
    label cellID;
    scalar weight;
};
typedef DynamicList<cellInfo> cellInfoList;

template<class Type>
wordList createBoundaryTypes
(
    const GeometricField<Type, fvPatchField, volMesh>& src,
    const fvMesh& targetMesh
)
{
    const typename GeometricField<Type, fvPatchField, volMesh>::Boundary&
        srcBoundary = src.boundaryField();

    HashTable<word> srcBoundaryTypes;
    forAll(srcBoundary, patchi)
    {
        if (!isA<genericFvPatchField<Type>>(srcBoundary[patchi]))
        {
            srcBoundaryTypes.insert
            (
                srcBoundary[patchi].patch().name(),
                srcBoundary[patchi].type()
            );
        }
        else
        {
            FatalErrorInFunction
                << "Unknown patch type for patch "
                << srcBoundary[patchi].patch().name()
                << " for " << src.name() << nl
                << "Perhaps the library is missing?" << nl
                << "include the necessary library with"
                << " \'libs (\"lib*.so\")\'" << nl
                << " in the controlDict" << endl
                << abort(FatalError);
        }
    }

    wordList targetBoundaryTypes
    (
        targetMesh.boundary().size(),
        calculatedFvPatchField<Type>::typeName
    );
    forAll(targetMesh.boundary(), patchi)
    {
        if (srcBoundaryTypes.found(targetMesh.boundaryMesh()[patchi].name()))
        {
            targetBoundaryTypes[patchi] =
                srcBoundaryTypes[targetMesh.boundaryMesh()[patchi].name()];
        }
    }
    return targetBoundaryTypes;
}


template<class Type>
void mapVolFields
(
    const PtrList<fvMesh>& sourceMeshes,
    const fvMesh& targetMesh,
    const List<cellInfoList>& cellMap,
    const List<cellInfoList>& extendedCellMap,
    const IOobjectList& objects,
    const tensorField& R,
    const HashSet<word>& mapFields,
    const bool store = false
)
{
    typedef GeometricField<Type, fvPatchField, volMesh> fieldType;
    IOobjectList fields = objects.lookupClass(fieldType::typeName);
    forAllIter(IOobjectList, fields, fieldIter)
    {
        IOobject fieldTargetIOobject
        (
            fieldIter()->name(),
            targetMesh.time().timeName(),
            targetMesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        );

        bool mapField = false;

        setParRun(false);
        PtrList<fieldType> fieldSources(sourceMeshes.size());
        forAll(sourceMeshes, proci)
        {
            fieldSources.set
            (
                proci,
                new fieldType
                (
                    IOobject
                    (
                        fieldIter()->name(),
                        sourceMeshes[proci].time().timeName(),
                        sourceMeshes[proci],
                        IOobject::MUST_READ
                    ),
                    sourceMeshes[proci]
                )
            );
        }
        resetParRun();

        autoPtr<fieldType> fieldTargetPtr;
        UautoPtr<const List<cellInfoList>> mapPtr;
        bool exists = false;
        if (targetMesh.foundObject<fieldType>(fieldIter()->name()))
        {
            mapField = true;
            fieldTargetPtr.set
            (
                &targetMesh.lookupObjectRef<fieldType>
                (
                    fieldIter()->name()
                )
            );
            mapPtr.set(&cellMap);
            exists = true;
        }
        else if (fieldTargetIOobject.typeHeaderOk<fieldType>(true))
        {
            mapField = true;
            fieldTargetPtr.set
            (
                new fieldType
                (
                    fieldTargetIOobject,
                    targetMesh
                )
            );
            mapPtr.set(&cellMap);
        }
        else if (mapFields.found(fieldTargetIOobject.name()))
        {
            mapField = true;
            fieldTargetIOobject.readOpt() = IOobject::NO_READ;
            fieldType* fieldPtr =
                new fieldType
                (
                    fieldTargetIOobject,
                    targetMesh,
                    dimensioned<Type>
                    (
                        "0",
                        fieldSources[0].dimensions(),
                        pTraits<Type>::zero
                    ),
                    createBoundaryTypes<Type>(fieldSources[0], targetMesh)
                );
            if (store)
            {
                fieldPtr->store(fieldPtr);
                exists = true;
            }
            fieldTargetPtr.set(fieldPtr);
            mapPtr.set(&extendedCellMap);
        }

        if (mapField)
        {
            Info<< "    mapping " << fieldIter()->name() << endl;

            // Read fieldTarget
            fieldType& fieldTarget = fieldTargetPtr();

            const List<cellInfoList>& map = mapPtr();

            forAll(map, celli)
            {
                if (map[celli].size())
                {
                    fieldTarget[celli] = Zero;
                    Type vSum = Zero;
                    scalar sumW = 0.0;
                    forAll(map[celli], j)
                    {
                        const cellInfo& info = map[celli][j];
                        vSum += info.weight*fieldSources[info.proc][info.cellID];
                        sumW += info.weight;
                    }
                    fieldTarget[celli] = transform(R[celli], vSum/sumW);
                }
            }
            forAll(fieldTarget.boundaryField(), patchi)
            {
                fieldTarget.boundaryFieldRef()[patchi] =
                    fieldTarget.boundaryField()[patchi].patchInternalField();
            }
            if (!exists)
            {
                fieldTarget.write();
            }
            else
            {
                // Remove the pointer because it should not be deleted
                // here
                fieldTargetPtr.ptr();
            }
        }

#ifdef FULLDEBUG
        else
        {
            Info<< "    Not mapping " << fieldIter()->name() << nl
                << "         Add to \"additionalFields\" if you would "
                << "like to include it" << endl;
        }
#endif
    }
}


void mapFields
(
    const PtrList<fvMesh>& sourceMeshes,
    const fvMesh& targetMesh,
    const List<cellInfoList>& cellMap,
    const List<cellInfoList>& extendedCellMap,
    const tensorField& R,
    const HashSet<word>& additionalFields,
    const bool store = false
)
{
    Info<< "Mapping fields" << endl;
    IOobjectList objects(sourceMeshes[0], sourceMeshes[0].time().timeName());

    mapVolFields<scalar>
    (
        sourceMeshes,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields,
        store
    );
    mapVolFields<vector>
    (
        sourceMeshes,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields,
        store
    );
    mapVolFields<sphericalTensor>
    (
        sourceMeshes,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields,
        store
    );
    mapVolFields<symmTensor>
    (
        sourceMeshes,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields,
        store
    );
    mapVolFields<tensor>
    (
        sourceMeshes,
        targetMesh,
        cellMap,
        extendedCellMap,
        objects,
        R,
        additionalFields,
        store
    );
    Info<< endl;
}


Foam::Pair<Foam::vector> calculateAxis(const fvMesh& mesh)
{
    Pair<vector> axis(vector::one, Zero);
    vector& rAxis = axis[0];
    vector& yAxis = axis[1];

    List<vector> foundAxis;
    forAll(mesh.boundaryMesh(), patchi)
    {
        if (isA<wedgePolyPatch>(mesh.boundaryMesh()[patchi]))
        {
            const wedgePolyPatch& wp = dynamicCast<const wedgePolyPatch>
            (
                mesh.boundaryMesh()[patchi]
            );
            bool found = false;
            vector a = cmptMag(wp.axis());
            forAll(foundAxis, ai)
            {
                if (mag(a - foundAxis[ai]) < 1e-6)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                foundAxis.append(a);
                rAxis -= cmptMag(wp.centreNormal()) + a;
                yAxis += a;
            }
        }
    }
    forAll(yAxis, cmpti)
    {
        rAxis[cmpti] = min(rAxis[cmpti]*pos(rAxis[cmpti] - 1e-6), 1.0);
        yAxis[cmpti] = min(yAxis[cmpti]*pos(yAxis[cmpti] - 1e-6), 1.0);
    }
    return axis;
}


void calcMapAndR
(
    const PtrList<indexedOctree<treeDataCell>>& icos,
    const PtrList<fvMesh>& sourceMeshes,
    const fvMesh& targetMesh,
    const scalar& maxR,
    const vector& sourceCentre,
    const vector& targetCentre,
    const vector& rotationAxis,
    const vector& rAxis,
    List<cellInfoList>& cellMap,
    List<cellInfoList>& extendedCellMap,
    tensorField& R
)
{
    Info<< "Calulating map and rotation tensors" << endl;
    label nSourceD = sourceMeshes[0].nGeometricD();
    vector sourceD(sourceMeshes[0].geometricD());
    vector targetD(targetMesh.geometricD());

    pointField transformedPoints(targetMesh.points());
    forAll(transformedPoints, pointi)
    {
        // Get the position on the target mesh
        vector ptTarget =
            cmptMultiply
            (
                targetMesh.points()[pointi],
                targetD
            );

        // Offset from the center of the source mesh
        vector nTarget = (ptTarget - targetCentre);

        // Radius
        scalar r = mag(nTarget);

        // Offset from the source mesh center (only solved directions)
        vector nSource(Zero);
        if (nSourceD == 1)
        {
            nSource = r*rAxis;
        }
        else
        {
            scalar y = nTarget & rotationAxis;
            scalar x = mag((nTarget - y*rotationAxis));
            nSource = y*rotationAxis + x*rAxis;
        }

        // Actual point on the source mesh
        transformedPoints[pointi] = nSource + sourceCentre;
    }


    forAll(cellMap, celli)
    {
        const cell& c = targetMesh.cells()[celli];

        // Mapping has already been set
        if (cellMap[celli].size())
        {
            continue;
        }
        // Get the position on the target mesh
        vector ptTarget =
            cmptMultiply
            (
                targetMesh.cellCentres()[celli],
                targetD
            );

        // Offset from the center of the source mesh
        vector nTarget = (ptTarget - targetCentre);

        // Radius
        scalar r = mag(nTarget);

        // Offset from the source mesh center (only solved directions)
        vector nSource(Zero);
        if (nSourceD == 1)
        {
            nSource = r*rAxis;
        }
        else
        {
            scalar y = nTarget & rotationAxis;
            scalar x = mag((nTarget - y*rotationAxis));
            nSource = y*rotationAxis + x*rAxis;
        }

        // Actual point on the source mesh
        vector ptSource = nSource + sourceCentre;

        // Map from the source mesh to the target mesh
        // Keep track of the maximum volume (weight) for extended cell
        scalar maxV = -great;
        label eCellI = -1;
        scalar sumV = 0.0;
        cellInfoList& infos = cellMap[celli];
        if (r < maxR || maxR < 0)
        {
            forAll(sourceMeshes, proci)
            {
                if (targetMesh.bounds().overlaps(sourceMeshes[proci].bounds()))
                {
                    treeBoundBox bb(c.points(targetMesh.faces(), transformedPoints));
                    forAll(sourceD, cmpti)
                    {
                        if (sourceD[cmpti] < 0)
                        {
                            bb.min()[cmpti] = -great;
                            bb.max()[cmpti] = great;
                        }
                    }

                    // Find all cells in the transformed bound box and weight based
                    // on cell volume
                    // NOTE: Ideally this would be overlap volume but that requires more
                    // work
                    labelList sCells(icos[proci].findBox(bb));
                    forAll(sCells, cj)
                    {
                        const label cellj = sCells[cj];
                        const scalar V = sourceMeshes[proci].V()[cellj];
                        sumV += V;
                        infos.append({proci, cellj, V});

                        if (V > maxV)
                        {
                            maxV = V;
                            eCellI = cj;
                        }
                    }
                }
            }
        }

        // normalize weights
        if (sumV > 0)
        {
            forAll(infos, cj)
            {
                infos[cj].weight /= sumV;
            }
        }

        // Extend radius is the target point is outside of the source mesh
        scalar dist = great;
        forAll(sourceMeshes, proci)
        {
            pointIndexHit pIH = icos[proci].findNearest(ptSource, great);
            scalar curDist = mag(pIH.hitPoint() - ptSource);
            if (curDist < dist)
            {
                dist = curDist;
                extendedCellMap[celli](0) = {proci, pIH.index(), 1.0};
            }
        }
        if (r < maxR && !cellMap[celli].size())
        {
            cellMap[celli](0) = extendedCellMap[celli][0];
        }

        //- If mapping from a 2D case then there is only 1 rotation axis
        //  so to remove problems of rotating about the wrong axis we
        //  explicitly remove the axis direction from the directional
        //  vectors
        if (nSourceD == 2)
        {
            nTarget -= (nTarget & rotationAxis)*rotationAxis;
            nSource -= (nSource & rotationAxis)*rotationAxis;
        }

        // Normalise directions
        if (mag(nTarget) > small)
        {
            nTarget = nTarget/mag(nTarget);
        }
        if (mag(nSource) > small)
        {
            nSource = nSource/mag(nSource);
        }
        R[celli] = rotationTensor(nSource, nTarget);
    }
    Info<< endl;
}


//- Read and add fields to the database
template<class Type, template<class> class Patch, class Mesh>
void readGeoFields(const fvMesh& mesh, const IOobjectList& objects)
{
    typedef GeometricField<Type, Patch, Mesh> FieldType;

    IOobjectList fields = objects.lookupClass(FieldType::typeName);
    forAllIter(IOobjectList, fields, fieldIter)
    {
        if (!mesh.foundObject<FieldType>(fieldIter()->name()))
        {
            IOobject fieldTargetIOobject
            (
                fieldIter()->name(),
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            );

            if (fieldTargetIOobject.typeHeaderOk<FieldType>(true))
            {
                FieldType* fPtr
                (
                    new FieldType
                    (
                        fieldTargetIOobject,
                        mesh
                    )
                );
                fPtr->store(fPtr);
            }
        }
    }
}


//- Read and add fields to the database
template<class Type>
void readPointFields(const fvMesh& mesh, const IOobjectList& objects)
{
    typedef GeometricField<Type, pointPatchField, pointMesh> FieldType;
    IOobjectList fields = objects.lookupClass(FieldType::typeName);
    forAllIter(IOobjectList, fields, fieldIter)
    {
        if (!mesh.foundObject<FieldType>(fieldIter()->name()))
        {
            IOobject fieldTargetIOobject
            (
                fieldIter()->name(),
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            );

            if (fieldTargetIOobject.typeHeaderOk<FieldType>(true))
            {
                FieldType* fPtr
                (
                    new FieldType
                    (
                        fieldTargetIOobject,
                        pointMesh::New(mesh)
                    )
                );
                fPtr->store(fPtr);
            }
        }
    }
}


//- Read and add all fields to the database
void readAllFields(const fvMesh& mesh)
{
    // Get all fields present at the current time
    IOobjectList objects(mesh, mesh.time().timeName());

    readGeoFields<scalar, fvPatchField, volMesh>(mesh, objects);
    readGeoFields<vector, fvPatchField, volMesh>(mesh, objects);
    readGeoFields<symmTensor, fvPatchField, volMesh>(mesh, objects);
    readGeoFields<sphericalTensor, fvPatchField, volMesh>(mesh, objects);
    readGeoFields<tensor, fvPatchField, volMesh>(mesh, objects);

    readGeoFields<scalar, fvsPatchField, surfaceMesh>(mesh, objects);
    readGeoFields<vector, fvsPatchField, surfaceMesh>(mesh, objects);
    readGeoFields<symmTensor, fvsPatchField, surfaceMesh>(mesh, objects);
    readGeoFields<sphericalTensor, fvsPatchField, surfaceMesh>(mesh, objects);
    readGeoFields<tensor, fvsPatchField, surfaceMesh>(mesh, objects);

    readPointFields<scalar>(mesh, objects);
    readPointFields<vector>(mesh, objects);
    readPointFields<symmTensor>(mesh, objects);
    readPointFields<sphericalTensor>(mesh, objects);
    readPointFields<tensor>(mesh, objects);
}


void refine
(
    const PtrList<indexedOctree<treeDataCell>>& icos,
    const PtrList<fvMesh>& sourceMeshes,
    fvMesh& targetMesh,
    const scalar maxR,
    const vector& sourceCentre,
    const vector& targetCentre,
    const vector& rotationAxis,
    const vector& rAxis,
    const wordList& additionalFieldNames
)
{
    labelList cellMap(targetMesh.nCells(), -1);
    labelList extendedCellMap(targetMesh.nCells(), -1);
    tensorField R(targetMesh.nCells(), tensor::I);

    autoPtr<IOdictionary> refineDictPtr;
    {
        IOobject dynamicMeshDictIO
        (
            IOobject
            (
                "dynamicMeshDict",
                targetMesh.time().constant(),
                targetMesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            )
        );
        IOobject rotateFieldsDictIO
        (
            IOobject
            (
                "rotateFieldsDict",
                targetMesh.time().system(),
                targetMesh,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            )
        );
        if (dynamicMeshDictIO.typeHeaderOk<IOdictionary>(true))
        {
            refineDictPtr.set(new IOdictionary(dynamicMeshDictIO));
        }
        else
        {
            if (rotateFieldsDictIO.typeHeaderOk<IOdictionary>(true))
            {
                refineDictPtr.set(new IOdictionary(rotateFieldsDictIO));
            }
        }

        if (!refineDictPtr.valid())
        {
            WarningInFunction
                << "Refinement was specified, but neither " << nl
                << dynamicMeshDictIO.objectPath() << nl << " or " << nl
                << rotateFieldsDictIO.objectPath() << nl << " was found."
                << "Skipping." << endl;

            return;
        }
    }
    const IOdictionary& refineDict(refineDictPtr());

    autoPtr<errorEstimator> error
    (
        errorEstimator::New(targetMesh, refineDict)
    );
    error->setForce(true);
    autoPtr<fvMeshRefiner> refiner
    (
        fvMeshRefiner::New
        (
            targetMesh,
            refineDict,
            true
        )
    );

    readAllFields(targetMesh);

    Info<< "Begining refinement iterations" << nl << endl;
    bool good = true;
    bool lastIter = false;
    label iter = 0;
    while (good)
    {
        if (iter++ > 10)
        {
            lastIter = true;
        }

        if (lastIter)
        {
            good = false;
        }

        Info<<"Iteration " << iter << endl;

        List<cellInfoList> cellMap(targetMesh.nCells());
        List<cellInfoList> extendedCellMap(targetMesh.nCells());
        tensorField R(targetMesh.nCells(), tensor::I);

        calcMapAndR
        (
            icos,
            sourceMeshes,
            targetMesh,
            maxR,
            sourceCentre,
            targetCentre,
            rotationAxis,
            rAxis,
            cellMap,
            extendedCellMap,
            R
        );

        // Map fields from the source mesh to the target mesh
        mapFields
        (
            sourceMeshes,
            targetMesh,
            cellMap,
            extendedCellMap,
            R,
            additionalFieldNames,
            true
        );
        if (!lastIter)
        {
            error->update();
            lastIter =
                !refiner->refine(error->error(), error->maxRefinement());
        }

        Info<< "ExecutionTime = " << targetMesh.time().elapsedCpuTime() << " s"
            << "  ClockTime = " << targetMesh.time().elapsedClockTime() << " s"
            << nl << endl;
    }
    refiner->write();

    Info<< "Final target mesh size: " << targetMesh.nCells() << nl
        << endl;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    //- Add options
    argList::addNote
    (
        "Rotational extrusion and map volume fields from one mesh to another\n"
    );
    argList::validArgs.append("sourceCase");

    argList::addOption
    (
        "sourceTime",
        "scalar|'latestTime'",
        "specify the source time"
    );
    argList::addOption
    (
        "sourceRegion",
        "word",
        "specify the source region"
    );
    argList::addOption
    (
        "targetRegion",
        "word",
        "specify the target region"
    );
    argList::addBoolOption
    (
        "parallelSource",
        "the source is decomposed"
    );
    argList::addBoolOption
    (
        "extend",
        "Use the closest cell value if a given cell is outside of the mesh"
    );
    argList::addOption
    (
        "maxR",
        "scalar|'maximum radius'",
        "Cut off radius to map"
    );
    argList::addOption
    (
        "centre",
        "vector|'(0 0 0)'",
        "Location of center in the target mesh"
    );
    argList::addOption
    (
        "additionalFields",
        "wordList|'(rho U)'",
        "List of additional fields to map"
    );
    argList::addBoolOption
    (
        "uniform",
        "Copy uniform objects (not time)"
    );
    argList::addBoolOption
    (
        "refine",
        "Iteratively map, rotate and refine the mesh"
    );
    argList::addBoolOption
    (
        "tets",
        "Use cell tet decomposition"
    );
    #include "addRegionOption.H"

    #include "setRootCase.H"
    parRun = Pstream::parRun();

    fileName rootDirTarget(args.rootPath());
    fileName caseDirTarget(args.globalCaseName());

    fileName casePath = args[1];
    const fileName rootDirSource = casePath.path().toAbsolute();
    const fileName caseDirSource = casePath.name();

    Info<< "Source: " << casePath << " " << caseDirSource << endl;
    word sourceRegion = fvMesh::defaultRegion;
    if (args.optionFound("sourceRegion"))
    {
        sourceRegion = args["sourceRegion"];
        Info<< "Source region: " << sourceRegion << endl;
    }

    Info<< "Target: " << rootDirTarget << " " << caseDirTarget << endl;
    word targetRegion = fvMesh::defaultRegion;
    if (args.optionFound("targetRegion"))
    {
        targetRegion = args["targetRegion"];
        Info<< "Target region: " << targetRegion << endl;
    }

    const bool parallelSource = args.optionFound("parallelSource");

    scalar maxR(-1);
    if (args.optionFound("maxR"))
    {
        maxR = args.optionRead<scalar>("maxR");
        Info<< "Maximum distance from target centre is " << maxR << endl;
    }
    else if (args.optionFound("extend"))
    {
        maxR = great;
        Info<< "Extending mapping to the edge of the domain" << endl;
    }

    wordList additionalFieldNames;
    if (args.optionFound("additionalFields"))
    {
        additionalFieldNames =
            args.optionRead<wordList>("additionalFields");
    }
    HashSet<word> additionalFields(additionalFieldNames);

    bool copyUniform = args.optionFound("uniform");


    const string caseDirOrig = getEnv("FOAM_CASE");
    const string caseNameOrig = getEnv("FOAM_CASENAME");

    Time targetRunTime(Foam::Time::controlDictName, args);
    fvMesh targetMesh
    (
        IOobject
        (
            targetRegion,
            targetRunTime.timeName(),
            targetRunTime,
            IOobject::MUST_READ
        )
    );
    Info<<"Created target mesh"<<endl;

    const polyMesh::cellDecomposition decompMode =
        args.optionFound("tets") ? polyMesh::CELL_TETS : polyMesh::FACE_DIAG_TRIS;

    PtrList<Time> sourceRunTimes;
    PtrList<fvMesh> sourceMeshes;
    label nSourceCells = 0;

    // Create source case argList
    argList sourceArgs(args);
    const_cast<ParRunControl&>(sourceArgs.parRunControl()) = ParRunControl();

    if (parallelSource)
    {
        label nProcs = fileHandler().nProcs(rootDirSource/caseDirSource);
        reduce(nProcs, maxOp<label>());

        setParRun(false);
        sourceRunTimes.setSize(nProcs);
        sourceMeshes.setSize(nProcs);

        setEnv("FOAM_CASE", rootDirSource/caseDirSource, true);
        setEnv("FOAM_CASENAME", caseDirSource, true);
        for (int proci=0; proci < nProcs; proci++)
        {
            sourceRunTimes.set
            (
                proci,
                new Time
                (
                    rootDirSource,
                    caseDirSource/fileName(word("processor") + name(proci))
                )
            );
            Time& runTimeSource = sourceRunTimes[proci];
            const_cast<dictionary&>(runTimeSource.controlDict()) =
                targetRunTime.controlDict();
            #include "setTimeIndex.H"

            sourceMeshes.set
            (
                proci,
                new fvMesh
                (
                    IOobject
                    (
                        sourceRegion,
                        runTimeSource.timeName(),
                        runTimeSource
                    )
                )
            );
            nSourceCells += sourceMeshes[proci].nCells();
        }
        resetParRun();

        Info<< nl << "Read " << nProcs << " source processor meshes" << endl;
    }
    else
    {
        sourceRunTimes.setSize(1);
        sourceMeshes.setSize(1);

        sourceRunTimes.set
        (
            0,
            new Time
            (
                Time::controlDictName,
                rootDirSource,
                caseDirSource
            )
        );
        Time& runTimeSource = sourceRunTimes[0];
        #include "setTimeIndex.H"

        sourceMeshes.set
        (
            0,
            new fvMesh
            (
                IOobject
                (
                    sourceRegion,
                    runTimeSource.timeName(),
                    runTimeSource
                )
            )
        );
        Info<< "Created source mesh\n" << endl;
        nSourceCells += sourceMeshes[0].nCells();
    }
    setEnv("FOAM_CASE", caseDirOrig, true);
    setEnv("FOAM_CASENAME", caseNameOrig, true);

    Info<< "\nSource time: " << sourceRunTimes[0].value()
        << "\nTarget time: " << targetRunTime.value()
        << nl << endl;

    vector sourceSumCV = Zero;
    scalar sourceSumV = 0.0;
    PtrList<indexedOctree<treeDataCell>> icos(sourceMeshes.size());
    setParRun(false);

    forAll(sourceMeshes, proci)
    {
        const fvMesh& sourceMesh = sourceMeshes[proci];
        sourceSumCV += sum(sourceMesh.C()*sourceMesh.V()).value();
        sourceSumV += sum(sourceMesh.V()).value();

        treeBoundBox meshBb(sourceMesh.bounds());

        // Calculate typical cell related size to shift bb by.
        scalar typDim = meshBb.avgDim()/(2.0*Foam::cbrt(scalar(sourceMesh.nCells())));

        treeBoundBox shiftedBb
        (
            meshBb.min(),
            meshBb.max() + vector(typDim, typDim, typDim)
        );

        icos.set
        (
            proci,
            new indexedOctree<treeDataCell>
            (
                treeDataCell(true, sourceMesh, decompMode),
                shiftedBb,
                10,         // maxLevel
                100,        // leafsize
                10.0        // duplicity
            )
        );
        sourceMesh.tetBasePtIs();
    }
    resetParRun();
    Info<< "created source meshes" << nl << endl;

    Pair<vector> sourceAxis(calculateAxis(sourceMeshes[0]));
    Pair<vector> targetAxis(calculateAxis(targetMesh));
    vector rotationAxis = sourceAxis[1] - targetAxis[1];
    vector rAxis = sourceAxis[0];

    vector sourceCentre = cmptMultiply(sourceSumCV, sourceAxis[1])/sourceSumV;
    vector targetCentre(sourceCentre);
    if (args.optionFound("centre"))
    {
        targetCentre = args.optionRead<vector>("centre");
    }

    Info<< "Source centre: " << sourceCentre << nl
        << "Target centre: " << targetCentre << endl;

    Info<< "Source mesh size: " << nSourceCells << endl;

    if (!args.optionFound("refine"))
    {
        Info<< "Target mesh size: " << targetMesh.nCells() << nl << endl;
        List<cellInfoList> cellMap(targetMesh.nCells());
        List<cellInfoList> extendedCellMap(targetMesh.nCells());
        tensorField R(targetMesh.nCells(), tensor::I);

        calcMapAndR
        (
            icos,
            sourceMeshes,
            targetMesh,
            maxR,
            sourceCentre,
            targetCentre,
            rotationAxis,
            rAxis,
            cellMap,
            extendedCellMap,
            R
        );

        // Map fields from the source mesh to the target mesh
        mapFields
        (
            sourceMeshes,
            targetMesh,
            cellMap,
            extendedCellMap,
            R,
            additionalFieldNames
        );
    }
    else
    {
        refine
        (
            icos,
            sourceMeshes,
            targetMesh,
            maxR,
            sourceCentre,
            targetCentre,
            rotationAxis,
            rAxis,
            additionalFieldNames
        );
        targetRunTime.writeNow();
    }

    if (copyUniform)
    {
        fileName local = "uniform";
        fileName path = targetMesh.time().timePath();

        IOobjectList uniformObjects
        (
            sourceMeshes[0],
            sourceRunTimes[0].timeName()/local
        );
        forAllConstIter
        (
            IOobjectList,
            uniformObjects,
            iter
        )
        {
            fileName name = iter()->name();
            if (name != "time")
            {
                fileName srcPath = iter()->objectPath();
                cp
                (
                    iter()->objectPath(),
                    path/local/name
                );
            }
        }
    }
    Info<< nl << "Finished" << endl
        << "ExecutionTime = " << targetRunTime.elapsedCpuTime() << " s"
        << "  ClockTime = " << targetRunTime.elapsedClockTime() << " s" << nl
        << endl;

    return 0;
}


// ************************************************************************* //
