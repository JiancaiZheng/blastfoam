/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2020 OpenFOAM Foundation
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

Class
    Foam::totalThermoPressureFvPatchScalarField

Description
    This boundary condition provides a total pressure condition.  Four
    variants are possible:

    1. incompressible subsonic:
        \f[
            p_p = p_0 - 0.5 |U|^2
        \f]
        where
        \vartable
            p_p     | incompressible pressure at patch [m^2/s^2]
            p_0     | incompressible total pressure [m^2/s^2]
            U       | velocity [m/s]
        \endvartable

    2. compressible subsonic:
        \f[
            p_p = p_0 - 0.5 \rho |U|^2
        \f]
        where
        \vartable
            p_p     | pressure at patch [Pa]
            p_0     | total pressure [Pa]
            \rho    | density [kg/m^3]
            U       | velocity [m/s]
        \endvartable

    3. compressible transonic (\f$\gamma = 1\f$):
        \f[
            p_p = \frac{p_0}{1 + 0.5 \psi |U|^2}
        \f]
        where
        \vartable
            p_p     | pressure at patch [Pa]
            p_0     | total pressure [Pa]
            \psi    | compressibility [m^2/s^2]
            U       | velocity [m/s]
        \endvartable

    4. compressible supersonic (\f$\gamma > 1\f$):
        \f[
            p_p = \frac{p_0}{(1 + 0.5 \psi G |U|^2)^{\frac{1}{G}}}
        \f]
        where
        \vartable
            p_p     | pressure at patch [Pa]
            p_0     | total pressure [Pa]
            \psi    | compressibility [m^2/s^2]
            G       | coefficient given by \f$\frac{\gamma}{1-\gamma}\f$ []
            \gamma  | ratio of specific heats (Cp/Cv) []
            U       | velocity [m/s]
        \endvartable

    The modes of operation are set by the dimensions of the pressure field
    to which this boundary condition is applied, the \c psi entry and the value
    of \c gamma:
    \table
        Mode                    | dimensions | psi   | gamma
        incompressible subsonic | p/rho      |       |
        compressible subsonic   | p          | none  |
        compressible transonic  | p          | psi   | 1
        compressible supersonic | p          | psi   | > 1
    \endtable

Usage
    \table
        Property     | Description                | Required | Default value
        U            | Velocity field name        | no       | U
        phi          | Flux field name            | no       | phi
        rho          | Density field name         | no       | rho
        psi          | Compressibility field name | no       | none
        gamma        | (Cp/Cv)                    | no       | 1
        p0           | Total pressure             | yes      |
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            totalThermoPressure;
        p0              uniform 1e5;
    }
    \endverbatim

See also
    Foam::dynamicPressureFvPatchScalarField
    Foam::fixedValueFvPatchField

SourceFiles
    totalThermoPressureFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef totalThermoPressureFvPatchScalarField_H
#define totalThermoPressureFvPatchScalarField_H

#include "dynamicThermoPressureFvPatchScalarField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
               Class totalThermoPressureFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class totalThermoPressureFvPatchScalarField
:
    public dynamicThermoPressureFvPatchScalarField
{
protected:

    // Protected Data

        //- Name of the velocity field
        const word UName_;

        //- Name of the flux field
        const word phiName_;


public:

    //- Runtime type information
    TypeName("totalThermoPressure");


    // Constructors

        //- Construct from patch and internal field
        totalThermoPressureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        totalThermoPressureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given totalThermoPressureFvPatchScalarField
        //  onto a new patch
        totalThermoPressureFvPatchScalarField
        (
            const totalThermoPressureFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Disallow copy without setting internal field reference
        totalThermoPressureFvPatchScalarField
        (
            const totalThermoPressureFvPatchScalarField&
        ) = delete;

        //- Copy constructor setting internal field reference
        totalThermoPressureFvPatchScalarField
        (
            const totalThermoPressureFvPatchScalarField&,
            const DimensionedField<scalar, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchScalarField> clone
        (
            const DimensionedField<scalar, volMesh>& iF
        ) const
        {
            return tmp<fvPatchScalarField>
            (
                new totalThermoPressureFvPatchScalarField(*this, iF)
            );
        }


    // Member Functions

        // Evaluation functions

            //- Update the coefficients associated with the patch field
            //  using the given patch total pressure and velocity fields
            void updateCoeffs
            (
                const scalarField& p0p,
                const vectorField& Up
            );

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
