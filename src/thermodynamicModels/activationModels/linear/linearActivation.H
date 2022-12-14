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

Class
    Foam::activationModels::linearActivation

Description
    Activation rate based on detonation velocity and location relative to
    detonation points

SourceFiles
    linearActivation.C

\*---------------------------------------------------------------------------*/

#ifndef linearActivation_H
#define linearActivation_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "activationModel.H"

namespace Foam
{
namespace activationModels
{

/*---------------------------------------------------------------------------*\
                           Class linearActivation Declaration
\*---------------------------------------------------------------------------*/

class linearActivation
:
    public activationModel
{
// Private data

        //- Detonation velocity
        dimensionedScalar vDet_;

        //- Is advection included
        Switch advection_;

        //- Ignition time
        volScalarField::Internal tIgn_;

        //- Return the time rate of chage of lambda
        virtual tmp<volScalarField> delta() const;

        //- Correct lambda field after advection
        virtual void correct();


public:

    //- Runtime type information
    TypeName("linear");

    // Constructor
    linearActivation
    (
        const fvMesh& mesh,
        const dictionary& dict,
        const word& phaseName
    );


    //- Destructor
    virtual ~linearActivation();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace activationModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
