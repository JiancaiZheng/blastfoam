/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2019 Synthetik Applied Technologies
     \\/     M anipulation  |
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
    Foam::activationModels::noneActivation

Description
    Material is fully reacted when the simulation starts

SourceFiles
    noneActivation.C

\*---------------------------------------------------------------------------*/

#ifndef noneActivation_H
#define noneActivation_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "activationModel.H"

namespace Foam
{
namespace activationModels
{

/*---------------------------------------------------------------------------*\
                           Class noneActivation Declaration
\*---------------------------------------------------------------------------*/

class noneActivation
:
    public activationModel
{
// Private data

        //- Does lambda need to be set to one
        mutable bool setLambda_;

        //- Return the time rate of chage of lambda
        virtual tmp<volScalarField> delta() const
        {
            NotImplemented;
            return lambda_;
        }



public:

    //- Runtime type information
    TypeName("none");

    // Constructor
    noneActivation
    (
        const fvMesh& mesh,
        const dictionary& dict,
        const word& phaseName
    );


    //- Destructor
    virtual ~noneActivation();


    // Member Functions

        //- Set pointers to mass and mass flux
        virtual void initializeModels()
        {}

        //- Solve sub-step stepi
        virtual void solve();

        //- Remove stored fields
        virtual void clearODEFields()
        {}

        //- Return initial energy source
        virtual tmp<volScalarField> initESource() const;

        //- Return energy source
        virtual tmp<volScalarField> ESource() const;

        //- Return the detonation energy
        virtual tmp<volScalarField> ddtLambda() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace activationModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
