/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019-2021
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

\*---------------------------------------------------------------------------*/

#include "timeIntegrator.H"
#include "timeIntegrationSystem.H"
#include "pointFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(timeIntegrator, 0);
    defineRunTimeSelectionTable(timeIntegrator, dictionary);
}


// * * * * * * * * * * * * * * Protected Functions * * * * * * * * * * * * * //

void Foam::timeIntegrator::updateAll()
{
    forAll(systems_, i)
    {
        systems_[i].update();
    }
}


void Foam::timeIntegrator::postUpdateAll()
{
    forAll(systems_, i)
    {
        Info<< "Post-updating " << systems_[i].name() << ":" << endl;
        systems_[i].postUpdate();
    }
    Info<< endl;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::timeIntegrator::timeIntegrator(const fvMesh& mesh, const label)
:
    regIOobject
    (
        IOobject
        (
            "globalTimeIntegrator",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        )
    ),
    mesh_(mesh),
    stepi_(0),
    f_(0),
    f0_(0),
    V0byV_
    (
        volScalarField::Internal
        (
            IOobject
            (
                "V0byV",
                mesh.time().timeName(),
                mesh
            ),
            mesh,
            1.0
        )
    ),
    curTimeIndex_(-1),
    modelsPtr_(nullptr),
    constraintsPtr_(nullptr),
    solveFields_()
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::timeIntegrator::~timeIntegrator()
{}


// * * * * * * * * * * * * * * * Public Functions  * * * * * * * * * * * * * //

void Foam::timeIntegrator::initialize()
{
    //- Determine if what fields need to be saved
    boolList saveOlds(as_.size(), false);
    boolList saveDeltas(bs_.size(), false);
    nSteps_ = as_.size();

    forAll(as_, i)
    {
        for (label j = 0; j < as_[i].size() - 1; j++)
        {
            saveOlds[j] = saveOlds[j] || mag(as_[i][j]) > small;
            saveDeltas[j] = saveDeltas[j] || mag(bs_[i][j]) > small;
        }
    }

    oldIs_.resize(nSteps_);
    deltaIs_.resize(nSteps_);
    label fi = 0;
    for (label i = 0; i < nSteps_; i++)
    {
        if (saveOlds[i])
        {
            oldIs_[i] = fi++;
        }
        else
        {
            oldIs_[i] = -1;
        }
    }
    nOld_ = fi;

    fi = 0;
    for (label i = 0; i < nSteps_; i++)
    {
        if (saveDeltas[i])
        {
            deltaIs_[i] = fi++;
        }
        else
        {
            deltaIs_[i] = -1;
        }
    }
    nDelta_ = fi;

    if (f_.size() == 0)
    {
        f_.resize(nSteps());
        f0_.resize(nSteps());
        f0_[0] = 0.0;
        f_[0] = sum(bs_[0]);
        for (label stepi = 2; stepi <= nSteps(); stepi++)
        {
            scalarList ts(stepi+1, 0.0);
            scalarList dts(stepi, 0.0);
            forAll(dts, i)
            {
                dts[i] = sum(bs_[i]);
            }
            ts[1] = dts[0];

            for (label i = 1; i < stepi; i++)
            {
                for (label j = 0; j < as_[i].size(); j++)
                {
                    ts[i+1] += as_[i][j]*ts[j];
                }
                ts[i+1] += dts[i];
            }
            f0_[stepi-1] = ts.last() - dts.last();
            f_[stepi-1] = f0_[stepi-1] + sum(bs_[stepi-1]);
        }
    }
}

void Foam::timeIntegrator::addSystem(timeIntegrationSystem& system)
{
    label oldSize = systems_.size();
    systems_.resize(oldSize + 1);
    systems_.set(oldSize, &system);
}


void Foam::timeIntegrator::createModels() const
{
    modelsPtr_.set(&fvModels::New(const_cast<fvMesh&>(mesh_)));
    constraintsPtr_.set(&fvConstraints::New(mesh_));

    const PtrList<fvModel>& models(modelsPtr_());
    forAll(models, modeli)
    {
        wordList fields(models[modeli].addSupFields());
        forAll(fields, fieldi)
        {
            if (!solveFields_.found(fields[fieldi]))
            {
                solveFields_.append(fields[fieldi]);
            }
        }
    }

    const PtrList<fvConstraint>& constraints(constraintsPtr_());
    forAll(constraints, modeli)
    {
        wordList fields(constraints[modeli].constrainedFields());
        forAll(fields, fieldi)
        {
            if (!solveFields_.found(fields[fieldi]))
            {
                solveFields_.append(fields[fieldi]);
            }
        }
    }
}


void Foam::timeIntegrator::preUpdateMesh()
{
    forAll(systems_, i)
    {
        systems_[i].preUpdateMesh();
    }

    if (modelsPtr_.valid())
    {
        modelsPtr_->preUpdateMesh();
    }
}


void Foam::timeIntegrator::integrate()
{
    if (mesh_.time().timeIndex() == curTimeIndex_)
    {
        reset();
    }
    curTimeIndex_ = mesh_.time().timeIndex();

    // Update and store original fields
    for (stepi_ = 1; stepi_ <= as_.size(); stepi_++)
    {
        Info<< this->type() << ": step " << stepi_ << endl;

        // Set use a linear change in volume
        // All fields are scaled according to the true volume
        if (mesh_.moving())
        {
            V0byV_ = (mesh_.V0()*(1.0 - f0()) + f0()*mesh_.V())/mesh_.V();
        }

        this->updateAll();
        forAll(systems_, i)
        {
            Info<< "Solving " << systems_[i].name() << ":" << endl;
            systems_[i].solve();
            Info<< endl;
        }
    }

    this->postUpdateAll();
    if (modelsPtr_.valid())
    {
        modelsPtr_->correct();
    }
    stepi_ = 0;
}


void Foam::timeIntegrator::clear()
{
    forAll(systems_, i)
    {
        systems_[i].clear();
    }

    clearOldFields(oldScalarFields_);
    clearOldFields(oldVectorFields_);
    clearOldFields(oldSphTensorFields_);
    clearOldFields(oldSymmTensorFields_);
    clearOldFields(oldTensorFields_);
    clearDeltaFields(deltaScalarFields_);
    clearDeltaFields(deltaVectorFields_);
    clearDeltaFields(deltaSphTensorFields_);
    clearDeltaFields(deltaSymmTensorFields_);
    clearDeltaFields(deltaTensorFields_);
}


void Foam::timeIntegrator::reset()
{
    DebugInfo<<"Resetting fields to old time"<<endl;
    resetFields<volScalarField>();
    resetFields<volVectorField>();
    resetFields<volSymmTensorField>();
    resetFields<volSphericalTensorField>();
    resetFields<volTensorField>();

    resetFields<surfaceScalarField>();
    resetFields<surfaceVectorField>();
    resetFields<surfaceSymmTensorField>();
    resetFields<surfaceSphericalTensorField>();
    resetFields<surfaceTensorField>();

    resetFields<pointScalarField>();
    resetFields<pointVectorField>();
    resetFields<pointSymmTensorField>();
    resetFields<pointSphericalTensorField>();
    resetFields<pointTensorField>();
}


Foam::scalar Foam::timeIntegrator::f() const
{
    return f_[stepi_-1];
}

Foam::scalar Foam::timeIntegrator::f0() const
{
    return f0_[stepi_ - 1];
}


bool Foam::timeIntegrator::finalStep() const
{
    return stepi_ == as_.size();
}



// ************************************************************************* //
