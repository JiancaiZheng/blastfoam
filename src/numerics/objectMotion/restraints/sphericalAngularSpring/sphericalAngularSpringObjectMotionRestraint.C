/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "sphericalAngularSpringObjectMotionRestraint.H"
#include "addToRunTimeSelectionTable.H"
#include "movingObject.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace objectMotionRestraints
{
    defineTypeNameAndDebug(sphericalAngularSpring, 0);

    addToRunTimeSelectionTable
    (
        objectMotionRestraint,
        sphericalAngularSpring,
        motion
    );
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::objectMotionRestraints::sphericalAngularSpring::
sphericalAngularSpring
(
    const word& name,
    const dictionary& dict
)
:
    objectMotionRestraint(name, dict),
    refQ_(),
    stiffness_(),
    damping_()
{
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::objectMotionRestraints::sphericalAngularSpring::
~sphericalAngularSpring()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::objectMotionRestraints::sphericalAngularSpring::restrain
(
    const movingObject& motion,
    vector& restraintPosition,
    vector& restraintForce,
    vector& restraintMoment
) const
{
    restraintMoment = Zero;

    for (direction cmpt=0; cmpt<vector::nComponents; cmpt++)
    {
        vector axis = Zero;
        axis[cmpt] = 1;

        vector refDir = Zero;
        refDir[(cmpt + 1) % 3] = 1;

        vector newDir = motion.orientation() & refDir;

        axis = (refQ_ & axis);
        refDir = (refQ_ & refDir);
        newDir -= (axis & newDir)*axis;

        restraintMoment += -stiffness_*(refDir ^ newDir);
    }

    restraintMoment += -damping_*motion.omega();

    restraintForce = Zero;

    // Not needed to be altered as restraintForce is zero, but set to
    // centreOfRotation to be sure of no spurious moment
    restraintPosition = motion.centreOfRotation();

    if (motion.report())
    {
        Info<< " moment " << restraintMoment
            << endl;
    }
}


bool Foam::objectMotionRestraints::sphericalAngularSpring::read
(
    const dictionary& dict
)
{
    objectMotionRestraint::read(dict);

    refQ_ = coeffDict_.lookupOrDefault<tensor>("referenceOrientation", I);

    if (mag(mag(refQ_) - sqrt(3.0)) > 1e-9)
    {
        FatalErrorInFunction
            << "referenceOrientation " << refQ_ << " is not a rotation tensor. "
            << "mag(referenceOrientation) - sqrt(3) = "
            << mag(refQ_) - sqrt(3.0) << nl
            << exit(FatalError);
    }

    coeffDict_.lookup("stiffness") >> stiffness_;
    coeffDict_.lookup("damping") >> damping_;

    return true;
}


void Foam::objectMotionRestraints::sphericalAngularSpring::write
(
    Ostream& os
) const
{
    writeEntry(os, "referenceOrientation", refQ_);

    writeEntry(os, "stiffness", stiffness_);

    writeEntry(os, "damping", damping_);
}


// ************************************************************************* //
