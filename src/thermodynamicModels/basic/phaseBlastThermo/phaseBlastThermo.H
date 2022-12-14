/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
2020-04-02 Jeff Heylmun:    Added simple class to derive fluid and solid thermos
                            from
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
    Foam::phaseBlastThermo

Description
    General thermodynamic class for a fluid/solid

SourceFiles
    phaseBlastThermo.C

\*---------------------------------------------------------------------------*/

#ifndef phaseBlastThermo_H
#define phaseBlastThermo_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "volFields.H"
#include "dictionary.H"
#include "wordIOList.H"
#include "timeIntegrationSystem.H"
#include "regIOobject.H"
#include "OSspecific.H"

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class phaseBlastThermo Declaration
\*---------------------------------------------------------------------------*/

class phaseBlastThermo
:
    public timeIntegrationSystem
{
protected:

    // Protected data

        //- Name of master thermo class
        word masterName_;

        //- Names of phase
        word phaseName_;

        //- Phase density
        volScalarField& rho_;

        //- Reference to temperature
        const volScalarField& T_;

        //- Reference to internal energy
        const volScalarField& e_;

        //- Residual volume fraction
        dimensionedScalar residualAlpha_;

        //- Residual density
        dimensionedScalar residualRho_;

        //- Minimum temperature
        scalar TLow_ = 0.0;


        // Protected member functions

            //- Temperature [K]
            volScalarField& TRef()
            {
                NotImplemented;
                return rho_;
            }

            //- Internal energy [J/kg]
            volScalarField& heRef()
            {
                NotImplemented;
                return rho_;
            }

            //- Heat capacity at constant pressure [J/kg/K]
            volScalarField& CpRef()
            {
                NotImplemented;
                return rho_;
            }

            //- Heat capacity at constant volume [J/kg/K]
            volScalarField& CvRef()
            {
                NotImplemented;
                return rho_;
            }

            //- Thermal diffusivity for enthalpy of mixture [kg/m/s]
            volScalarField& alphaRef()
            {
                NotImplemented;
                return rho_;
            }

public:

    // Declare runtime construction

        declareRunTimeSelectionTable
        (
            autoPtr,
            phaseBlastThermo,
            dictionary,
            (
                const fvMesh& mesh,
                const dictionary& dict,
                const word& phaseName,
                const word& masterName
            ),
            (mesh, dict, phaseName, masterName)
        );


    //- Runtime type information
    TypeName("phaseBlastThermo");


    // Constructor

        //- Construct with input field
        phaseBlastThermo
        (
            const fvMesh& mesh,
            const dictionary& dict,
            const word& phaseName,
            const word& masterName = word::null
        );

        //- Initialize models
        virtual void initializeModels()
        {}


    //- Destructor
    virtual ~phaseBlastThermo();

    // Selectors

        static autoPtr<phaseBlastThermo> New
        (
            const fvMesh& mesh,
            const dictionary& dict,
            const word& phaseName,
            const word& masterName = word::null
        );


    // Static public functions

        // Directory to compile from
        static fileName templateDir()
        {
            return getEnv("BLAST_CODE_TEMPLATES");
        }

        //- Return the thermodynamic type name
        virtual word thermoName() const = 0;


    // Access functions

        //- Return the name of a property for this thermos phase
        word phasePropertyName(const word& n) const
        {
            return IOobject::groupName(n, phaseName_);
        }

        //- Return the name of the phase
        const word& phaseName() const
        {
            return phaseName_;
        }

        //- Return the name of the master phase name
        const word& masterName() const
        {
            return masterName_;
        }

        //- Return the residual alpha value
        const dimensionedScalar& residualAlpha() const
        {
            return residualAlpha_;
        }

        //- Return the residual density value
        const dimensionedScalar& residualRho() const
        {
            return residualRho_;
        }

        virtual dimensionedScalar residualAlphaRho() const
        {
            return residualAlpha()*residualRho();
        }

        //- Return constant reference to density field
        const volScalarField& rho() const
        {
            return rho_;
        }

        //- Return non-constant reference to density field
        volScalarField& rho()
        {
            return rho_;
        }

        //- Return constant reference to density field
        const volScalarField& he() const
        {
            return e_;
        }

        //- Return constant reference to density field
        const volScalarField& T() const
        {
            return T_;
        }


    // Member Functions

        //- Return energy source
        virtual tmp<volScalarField> ESource() const = 0;

        //- Return initial energy source
        virtual tmp<volScalarField> initESource() const = 0;

        //- Thermodynamic and transport functions

            //- Enthalpy/Internal energy
            //  for given pressure and temperature [J/kg]
            virtual tmp<volScalarField> he
            (
                const volScalarField& p,
                const volScalarField& T
            ) const = 0;

            //- Enthalpy/Internal energy for cell-set [J/kg]
            virtual tmp<scalarField> he
            (
                const scalarField& T,
                const labelList& cells
            ) const = 0;

            //- Enthalpy/Internal energy for patch [J/kg]
            virtual tmp<scalarField> he
            (
                const scalarField& T,
                const label patchi
            ) const = 0;

            //- Enthalpy/internal energy for a cell
            virtual scalar cellHE
            (
                const scalar T,
                const label celli
            ) const = 0;

            //- Enthalpy/internal energy for a patch and face
            virtual scalar patchFaceHE
            (
                const scalar T,
                const label patchi,
                const label facei
            ) const = 0;

            //- Sensible enthalpy [J/kg]
            virtual tmp<volScalarField> hs() const = 0;

            //- Sensible enthalpy
            //  for given pressure and temperature [J/kg]
            virtual tmp<volScalarField> hs
            (
                const volScalarField& p,
                const volScalarField& T
            ) const = 0;

            //- Sensible enthalpy for patch [J/kg]
            virtual tmp<scalarField> hs
            (
                const scalarField& T,
                const label patchi
            ) const = 0;

            //- Sensible enthalpy for cell-set [J/kg]
            virtual tmp<scalarField> hs
            (
                const scalarField& T,
                const labelList& cells
            ) const = 0;

            //- Absolute enthalpy [J/kg]
            virtual tmp<volScalarField> ha() const = 0;

            //- Absolute enthalpy
            //  for given pressure and temperature [J/kg]
            virtual tmp<volScalarField> ha
            (
                const volScalarField& p,
                const volScalarField& T
            ) const = 0;

            //- Absolute enthalpy for patch [J/kg/K]
            virtual tmp<scalarField> ha
            (
                const scalarField& T,
                const label patchi
            ) const = 0;

            //- Absolute enthalpy for cell-set [J/kg]
            virtual tmp<scalarField> ha
            (
                const scalarField& T,
                const labelList& cells
            ) const = 0;

            //- Enthalpy of formation [J/kg]
            virtual tmp<volScalarField> hc() const = 0;

            //- Flame temperature [K]
            virtual tmp<volScalarField> flameT() const = 0;

            //- Temperature from enthalpy/internal energy
            virtual tmp<volScalarField> THE() const = 0;

            //- Temperature from enthalpy/internal energy
            virtual tmp<volScalarField> THE
            (
                const volScalarField& h,
                const volScalarField& p,
                const volScalarField& T0    // starting temperature
            ) const = 0;

            //- Temperature from enthalpy/internal energy for cell-set
            virtual tmp<scalarField> THE
            (
                const scalarField& he,
                const scalarField& T0,      // starting temperature
                const labelList& cells
            ) const = 0;

            //- Temperature from enthalpy/internal energy for patch
            virtual tmp<scalarField> THE
            (
                const scalarField& he,
                const scalarField& T0,      // starting temperature
                const label patchi
            ) const = 0;

            //- Temperature from enthalpy/internal energy for celli
            virtual scalar cellTHE
            (
                const scalar he,
                const scalar T0,
                const label celli
            ) const = 0;

            //- Heat capacity at constant pressure for patch [J/kg/K]
            virtual tmp<scalarField> Cp
            (
                const scalarField&,
                const label
            ) const = 0;

            //- Heat capacity at constant pressure for a cell [J/kg/K]
            virtual scalar cellCp(const scalar, const label) const = 0;

            //- Heat capacity at constant volume for patch [J/kg/K]
            virtual tmp<scalarField> Cv
            (
                const scalarField&,
                const label
            ) const = 0;

            //- Heat capacity at constant volume for a cell [J/kg/K]
            virtual scalar cellCv(const scalar, const label) const = 0;

            //- Heat capacity at constant pressure/volume for patch [J/kg/K]
            virtual tmp<scalarField> Cpv
            (
                const scalarField&,
                const label
            ) const = 0;

            //- Heat capacity at constant pressure/volume for a cell [J/kg/K]
            virtual scalar cellCpv(const scalar, const label) const = 0;

            //- Heat capacity at constant pressure/volume for a patch and face [J/kg/K]
            virtual scalar patchFaceCpv
            (
                const scalar T,
                const label patchi,
                const label facei
            ) const = 0;

             //- Molecular weight [kg/kmol]
            virtual tmp<volScalarField> W() const = 0;

            //- Molecular weight for patch [kg/kmol]
            virtual tmp<scalarField> W(const label) const = 0;

            //- Molecular weight for cell [kg/kmol]
            virtual scalar cellW(const label) const = 0;


        //- Read residual values
        virtual void read(const dictionary& dict)
        {
            this->residualRho_.read(dict);
            this->residualAlpha_.read(dict);
        }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


#endif

// ************************************************************************* //
