/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2021-2022
     \\/     M anipulation  | Synthetik Applied Technologies
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

\*---------------------------------------------------------------------------*/

#include "rootSolver.H"
#include "univariateRootSolver.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::autoPtr<Foam::rootSolver> Foam::rootSolver::New
(
    const scalarMultivariateEquation& eqn,
    const dictionary& dict
)
{
    return New(dict.lookup<word>("solver"), eqn, dict);
}


Foam::autoPtr<Foam::rootSolver> Foam::rootSolver::New
(
    const word& rootSolverType,
    const scalarMultivariateEquation& eqn,
    const dictionary& dict
)
{
    if (isA<scalarEquation>(eqn))
    {
        return autoPtr<rootSolver>
        (
            univariateRootSolver::New
            (
                rootSolverType,
                dynamicCast<const scalarEquation>(eqn),
                dict
            ).ptr()
        );
    }

    Info<< "Selecting root solver: " << rootSolverType << endl;
    if (eqn.nDerivatives() <= 0)
    {
        dictionaryZeroConstructorTable::iterator cstrIter =
            dictionaryZeroConstructorTablePtr_->find(rootSolverType);

        if (cstrIter == dictionaryZeroConstructorTablePtr_->end())
        {
            FatalErrorInFunction
                << "Unknown root solver type "
                << rootSolverType << nl << nl
                << "Valid root solvers for no derivates are : " << endl
                << dictionaryZeroConstructorTablePtr_->sortedToc()
                << exit(FatalError);
        }
        return autoPtr<rootSolver>(cstrIter()(eqn, dict));
    }
    dictionaryOneConstructorTable::iterator cstrIter =
        dictionaryOneConstructorTablePtr_->find(rootSolverType);

    if (cstrIter == dictionaryOneConstructorTablePtr_->end())
    {
        FatalErrorInFunction
            << "Unknown root solver type "
            << rootSolverType << nl << nl
            << "Valid root solvers are : " << endl
            << dictionaryOneConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return autoPtr<rootSolver>(cstrIter()(eqn, dict));
}


// ************************************************************************* //
