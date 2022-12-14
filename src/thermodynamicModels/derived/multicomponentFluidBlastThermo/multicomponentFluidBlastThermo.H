/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
2020-04-02 Jeff Heylmun:    Modified class for a density based thermodynamic
                            class
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
    Foam::multicomponentFluidBlastThermo

Description
    Multicomponent fluid model with species transport

SourceFiles
    multicomponentFluidBlastThermo.C
    multicomponentFluidBlastThermos.C

\*---------------------------------------------------------------------------*/

#ifndef multicomponentFluidBlastThermo_H
#define multicomponentFluidBlastThermo_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "multicomponentBlastThermo.H"
#include "mixtureBlastThermo.H"

namespace Foam
{

template<class Thermo>
class multicomponentFluidBlastThermo
:
    public Thermo
{
    //- Calculate thermodynamic fields
    void calculate();


public:

    //- Runtime type information
    TypeName("multicomponent");

    // Constructor
    multicomponentFluidBlastThermo
    (
        const fvMesh& mesh,
        const dictionary& dict,
        const word& phaseName,
        const word& masterName = word::null
    );

    // Constructor
    multicomponentFluidBlastThermo
    (
        const HashPtrTable<typename Thermo::thermoType, word, string::hash>& thermoData,
        const fvMesh& mesh,
        const dictionary& dict,
        const word& phaseName,
        const word& masterName = word::null
    );

    //- Destructor
    virtual ~multicomponentFluidBlastThermo();


    // Member Functions

        //- Correct thermodynamic fields
        virtual void correct();

        //- Add contribution to mixture temperature
        virtual void calculate
        (
            const volScalarField& alpha,
            const volScalarField& he,
            const volScalarField& T,
            volScalarField& Cp,
            volScalarField& Cv,
            volScalarField& mu,
            volScalarField& alphah,
            volScalarField& pXiSum,
            volScalarField& XiSum
        );

        //- Add contribution to speed of sound
        virtual void calculateSpeedOfSound
        (
            const volScalarField& alpha,
            volScalarField& cSqrRhoXiSum
        );

        //- Pre update step
        virtual void update()
        {
            multicomponentBlastThermo::update();
        }

        //- Solve sub-step stepi
        virtual void solve()
        {
            multicomponentBlastThermo::solve();
        }

        //- Post update
        virtual void postUpdate()
        {
            multicomponentBlastThermo::postUpdate();
        }

        //- Return energy source
        virtual tmp<volScalarField> ESource() const;

        //- Return initial energy source
        virtual tmp<volScalarField> initESource() const;


    //- Thermodynamic and transport functions

        using Thermo::updateRho;

        //- Calculate density
        virtual void updateRho(const volScalarField& p);

        //- Calculate density
        virtual void updateRho
        (
            const volScalarField& alpha,
            const volScalarField& p
        );

        //- Calculate pressure for cell [Pa]
        virtual scalar cellpRhoT
        (
            const label celli,
            const bool limit = true
        ) const;

        //- Calculate Gamma = dpde
        virtual tmp<volScalarField> Gamma() const;

        //- Gamma = dpde for cell []
        virtual scalar cellGamma(const label celli) const;

        //- Derivative of pressure w.r.t density for cell
        virtual scalar celldpdRho(const label celli) const;

        //- Derivative of pressure w.r.t internal energy for cell
        virtual scalar celldpde(const label celli) const;

        //- Derivative of pressure w.r.t temperature for cell
        virtual scalar celldpdT(const label celli) const;

        //- Is the equation of state temperature based
        virtual bool temperatureBased() const
        {
            return Thermo::thermoType::temperatureBased();
        }

        //- Calculate internal energy from pressure and density
        virtual tmp<volScalarField> calce(const volScalarField& p) const;

        //- Calculate internal energy for a cell
        virtual scalar calcCelle(const scalar p, const label) const;

        //- Calculate the pressure
        virtual tmp<volScalarField> calcp() const;

        //- Calculate the speed of sound
        virtual tmp<volScalarField> calcSpeedOfSound() const;


        // Specie thermo properties

            using Thermo::baseThermo::p;

            //- Return the pressure for cell [Pa]
            virtual scalar cellp(const label celli) const
            {
                return this->p_[celli];
            }

            //- Pressure [kg/m/s^2]
            virtual scalar p
            (
                const label speciei,
                const scalar rho,
                const scalar e,
                const scalar T
            ) const;

            //- Density [kg/m^3]
            virtual tmp<volScalarField> p
            (
                const label speciei,
                const volScalarField& rho,
                const volScalarField& e,
                const volScalarField& T
            ) const;

            //- Return derivative of pressure w.r.t. density
            virtual scalar dpdRho
            (
                const label speciei,
                const scalar rho,
                const scalar e,
                const scalar T
            ) const;

            //- Return derivative of pressure w.r.t. Temperature
            virtual scalar dpdT
            (
                const label speciei,
                const scalar rho,
                const scalar e,
                const scalar T
            ) const;


        // Specie transport properties

            using Thermo::baseThermo::mu;

            //- Dynamic viscosity [kg/m/s]
            virtual scalar mu
            (
                const label speciei,
                const scalar p,
                const scalar T
            ) const;

            //- Dynamic viscosity [kg/m/s]
            virtual scalar mu
            (
                const label speciei,
                const scalar rho,
                const scalar e,
                const scalar T
            ) const;

            //- Dynamic viscosity [kg/m/s]
            virtual tmp<volScalarField> mu
            (
                const label speciei,
                const volScalarField& p,
                const volScalarField& T
            ) const;

            //- Dynamic viscosity [kg/m/s]
            virtual tmp<volScalarField> mu
            (
                const label speciei,
                const volScalarField& rho,
                const volScalarField& e,
                const volScalarField& T
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "multicomponentFluidBlastThermo.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
