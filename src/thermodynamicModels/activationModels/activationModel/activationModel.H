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
    Foam::activationModel

Description
    Base model to describe activtion of an energetic material

SourceFiles
    activationModel.C
    newactivationModel.C

\*---------------------------------------------------------------------------*/

#ifndef activationModel_H
#define activationModel_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "volFields.H"
#include "surfaceFields.H"
#include "dictionary.H"
#include "IOPtrList.H"
#include "timeIntegrationSystem.H"
#include "runTimeSelectionTables.H"

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class activationModel Declaration
\*---------------------------------------------------------------------------*/

class activationModel
:
    public timeIntegrationSystem
{
public:

    // Public class declaration

        // Declaration of detonation point class
        class detonationPoint
        :
            public vector
        {
            //- Has this point been activated
            mutable bool activated_;

            //- Has the point been printed
            mutable bool printedActivated_;

            //- Delay time
            scalar delay_;

            //- Radius of activation (0 = 1 cell)
            scalar radius_;

            //- List of activated cell indicies
            labelList cells_;

        public:

            //- Type name
            TypeName("detonationPoint");

            // Constructors

                //- Construct from components
                detonationPoint
                (
                    const vector& pt,
                    const scalar delay,
                    const scalar radius
                );

                //- Construct from Istream
                detonationPoint(Istream& is);


            //- Construct on freestore from Istream and return
            static autoPtr<detonationPoint> New(Istream& is);

            //- Clone the detonation point
            virtual autoPtr<detonationPoint> clone() const;

            virtual ~detonationPoint();

            // Public access

                //- Has the point been activated
                bool activated() const
                {
                    return activated_;
                }

                //- Has the point been activated
                bool& activated()
                {
                    return activated_;
                }

                //- Return delay
                scalar delay() const
                {
                    return delay_;
                }

                //- Return the activation radius
                scalar radius() const
                {
                    return radius_;
                }

                //- Check if the point will activate any cells or mass
                //  and set activated if time > delay
                bool check(const volScalarField&);

                //- Set activated cells by setting lambda
                void setActivated(volScalarField&, bool update) const;

            // Ostream operator

                friend Ostream& operator<<
                (
                    Ostream&,
                    const detonationPoint&
                );
        };

protected:

        //- Progress variable
        volScalarField lambda_;

        //- Detonation points
        IOPtrList<detonationPoint> detonationPoints_;

        //- Specific activation energy
        dimensionedScalar e0_;

        //- Exponent used to blend reacted and unreacted states (\lambda^m)
        scalar lambdaExp_;


        //- Reference to phase mass
        UautoPtr<const volScalarField> alphaRhoPtr_;

        //- Reference to phase mass flux
        UautoPtr<const surfaceScalarField> alphaRhoPhiPtr_;

        //- Maximum change in lambda per time step
        scalar maxDLambda_;

        //- Is the change in lambda limited
        bool limit_;

        //- Stored changes in lambda
        tmp<volScalarField> ddtLambda_;


    // Protected functions

        //- Return the time rate of chage of lambda
        virtual tmp<volScalarField> delta() const = 0;

        //- Correct lambda field after advection
        virtual void correct()
        {}

        //- Return the list of detonation points read from the
        //  input dictionary
        PtrList<detonationPoint> readDetonationPoints
        (
            const dictionary& dict,
            const volScalarField& alpha,
            const bool needDetonationPoints
        ) const;

        //- Return the center of centerOfMass
        //  Only valid for a single detonation point
        vector centerOfMass(const volScalarField& alpha) const;


public:

    //- Runtime type information
    TypeName("activationModel");


    // Declare runtime construction

        declareRunTimeSelectionTable
        (
            autoPtr,
            activationModel,
            dictionary,
            (
                const fvMesh& mesh,
                const dictionary& dict,
                const word& phaseName
            ),
            (mesh, dict, phaseName)
        );

    // Constructor
    activationModel
    (
        const fvMesh& mesh,
        const dictionary& dict,
        const word& phaseName,
        const bool needDetonationPoints
    );


    //- Destructor
    virtual ~activationModel();


    // Selectors

        static autoPtr<activationModel> New
        (
            const fvMesh& mesh,
            const dictionary& dict,
            const word& phaseName
        );


    // Member Functions

        //- Set pointers to mass and mass flux
        virtual void initializeModels();

        //- Update
        virtual void update()
        {}

        //- Solve sub-step stepi
        virtual void solve();

        //- Post update
        virtual void postUpdate()
        {}

        //- Return the specific detonation energy
        const dimensionedScalar& e0() const
        {
            return e0_;
        }

        //- Return const reference to progress variable
        const volScalarField& lambda() const
        {
            return lambda_;
        }

        //- Return lambda to the m power for blending
        inline tmp<volScalarField> lambdaPow() const
        {
            return pow(lambda_, lambdaExp_);
        }

        //- Return lambda to the m power for blending for patchi
        inline tmp<scalarField> lambdaPow(const label patchi) const
        {
            return pow(lambda_.boundaryField()[patchi], lambdaExp_);
        }

        //- Return ambda to the m power for blending for celli
        inline scalar cellLambdaPow(const label celli) const
        {
            return pow(lambda_[celli], lambdaExp_);
        }

        //- Return lambda to the m power for blending for patchi and facei
        inline scalar patchFaceLambdaPow
        (
            const label patchi,
            const label facei
        ) const
        {
            return pow(lambda_.boundaryField()[patchi][facei], lambdaExp_);
        }

        //- Return energy source
        virtual tmp<volScalarField> ddtLambda() const;

        //- Return initial energy source
        virtual tmp<volScalarField> initESource() const;

        //- Return energy source
        virtual tmp<volScalarField> ESource() const;
};


Ostream& operator<<(Ostream&, const activationModel::detonationPoint&);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
