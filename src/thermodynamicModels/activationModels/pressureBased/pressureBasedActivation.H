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
    Foam::activationModels::pressureBasedActivation

Description
    Activation rate based on local pressure

    References:
    \verbatim
        Kim, B., Park, J., Lee, K., Yoh, J. J. (2014).
        A reactive flow model for heavily aluminized
        cyclotrimethylene-trinitramine
        Journal of Applied Physics. 116. 1-9
    \endverbatim

SourceFiles
    pressureBasedActivation.C

\*---------------------------------------------------------------------------*/

#ifndef pressureBasedActivation_H
#define pressureBasedActivation_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "activationModel.H"

namespace Foam
{
namespace activationModels
{

/*---------------------------------------------------------------------------*\
                           Class pressureBasedActivation Declaration
\*---------------------------------------------------------------------------*/

class pressureBasedActivation
:
    public activationModel
{
// Private data

    //- Scale for pressure (Mpa = 1e-6)
    scalar pScale_;

    //- Ignition coefficients
    dimensionedScalar I_;
    scalar a_;
    scalar b_;
    scalar x_;
    scalar maxLambdaI_;
    bool needI_;

    //- First reaction rate constants
    dimensionedScalar G1_;
    scalar c_;
    scalar d_;
    scalar y_;
    scalar minLambda1_;
    scalar maxLambda1_;
    bool needG1_;

    //- Second reaction rate constants
    dimensionedScalar G2_;
    scalar e_;
    scalar f_;
    scalar z_;
    scalar minLambda2_;
    scalar maxLambda2_;
    bool needG2_;

    //- Name of pressure
    word pName_;

    //- Reference to total pressure
    const volScalarField& p_;

    //- Const reference to phase density
    const volScalarField& rho_;

    //- Reference density
    dimensionedScalar rho0_;

    //- Cutoff pressure
    dimensionedScalar pMin_;


    //- Return the time rate of chage of lambda
    virtual tmp<volScalarField> delta() const;


public:

    //- Runtime type information
    TypeName("pressureBased");

    // Constructor
    pressureBasedActivation
    (
        const fvMesh& mesh,
        const dictionary& dict,
        const word& phaseName
    );


    //- Destructor
    virtual ~pressureBasedActivation();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace activationModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
