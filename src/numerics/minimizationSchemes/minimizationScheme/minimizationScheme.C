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

#include "minimizationScheme.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(minimizationScheme, 0);
    defineRunTimeSelectionTable(minimizationScheme, dictionaryUnivariate);
    defineRunTimeSelectionTable(minimizationScheme, dictionaryMultivariate);
}

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool Foam::minimizationScheme::converged
(
    const scalarList& absErrors,
    const scalarList& relErrors,
    const scalarList& absTols,
    const scalarList& relTols
) const
{
    bool c = true;
    forAll(absErrors, i)
    {
        if (normalize_)
        {
            c =
                c
             && (
                    relErrors[i] < relTols[i]
                 || absErrors[i] < absTols[i]
                );
        }
        else
        {
            c = c && absErrors[i] < absTols[i];
        }
    }
    return c;
}


bool Foam::minimizationScheme::convergedXScale
(
    const scalarList& errors,
    const scalarList& s
) const
{
    forAll(xErrors_, i)
    {
        xErrors_[i] = mag(errors[i]);
        xRelErrors_[i] = xErrors_[i]/stabilise(mag(s[i]), small);
    }
    return converged(xErrors_, xRelErrors_, xTolerances_, xRelTolerances_);
}


bool Foam::minimizationScheme::convergedX
(
    const scalarList& x1,
    const scalarList& x2
) const
{
    forAll(xErrors_, i)
    {
        xErrors_[i] = mag(x2[i] - x1[i]);
        xRelErrors_[i] =
            xErrors_[i]/stabilise(min(mag(x1[i]), mag(x2[i])), small);
    }
    return converged(xErrors_, xRelErrors_, xTolerances_, xRelTolerances_);
}


bool Foam::minimizationScheme::convergedYScale
(
    const scalarList& errors,
    const scalarList& s
) const
{
    forAll(yErrors_, i)
    {
        yErrors_[i] = mag(errors[i]);
        yRelErrors_[i] = yErrors_[i]/stabilise(mag(s[i]), small);
    }
    return converged(yErrors_, yRelErrors_, yTolerances_, yRelTolerances_);
}


bool Foam::minimizationScheme::convergedY
(
    const scalarList& y1,
    const scalarList& y2
) const
{
    forAll(yErrors_, i)
    {

        yErrors_[i] = mag(y2[i] - y1[i]);
        yRelErrors_[i] =
            yErrors_[i]/stabilise(min(mag(y1[i]), mag(y2[i])), small);
    }
    return converged(yErrors_, yRelErrors_, yTolerances_, yRelTolerances_);
}


void Foam::minimizationScheme::printStepInformation
(
    const scalarList& vals
) const
{
    if (debug > 2)
    {
        DebugInfo<< "Step: " << stepi_ << ":" << nl
            << "    Errors (abs/rel): "
            << xErrors_ << ", " << xRelErrors_ << endl;
        if (checkY_)
        {
            Info<< "    Deltas (abs/rel): "
                << yErrors_ << ", " << yRelErrors_ << endl;
        }
        Info<< "    Minimums: " << vals << endl;
    }
}


void Foam::minimizationScheme::printFinalInformation
(
    const scalarList& vals
) const
{
    if (!debug)
    {
        return;
    }
    bool converged =
        max(xErrors_ - xTolerances_) <= 0.0
     || max(xRelErrors_ - xRelTolerances_) <= 0.0
     || max(yErrors_ - yTolerances_) <= 0.0
     || max(yRelErrors_ - yRelTolerances_) <= 0.0;
    if (converged)
    {
        Info<< "Converged in " << stepi_ << " iterations" << endl;
    }
    else if (stepi_ < maxSteps_)
    {
        WarningInFunction
            << "Did not converge due to bounds, tried "
            << stepi_ << " iterations" << endl;
    }
    else
    {
        WarningInFunction
            << "Did not converge in "
            << stepi_ << " iterations" << endl;
    }
    Info<< "    Final errors (abs/rel): "
        << xErrors_ << ", " << xRelErrors_ << endl;
    if (checkY_)
    {
        Info<< "    Final deltas (abs, rel): "
            << yErrors_ << ", " << yRelErrors_ << endl;
    }
    Info<< "    Minimums: " << vals << endl;
}


Foam::scalar Foam::minimizationScheme::alpha
(
    const scalarList& grad,
    const scalarList& gradOld
) const
{
    return 1.0;
}


Foam::scalar Foam::minimizationScheme::lineSearch
(
    const scalarList& x0,
    const scalarList& grad,
    const scalarList& gradOld,
    const label li,
    scalar& fx
) const
{
    if (debug > 3)
    {
        Info<< "Conducting line search" << endl;
    }

//     scalar alpha = norm(grad)/stabilise(norm(gradOld), small);
//     scalar alpha =
//         max
//         (
//             inner(grad, grad - gradOld)/stabilise(norm(gradOld), small),
//             0.0
//         );
    scalar a = alpha(grad, gradOld);
    const scalar fx0(fx);
    scalarField x1(x0 - a*grad);
    eqns_.limit(x1);
    fx = eqns_.fX(x1, li);

    label iter = 0;
    while (fx >= fx0 && iter++ < maxSteps_)
    {
        a *= tau_;
        x1 = x0 - a*grad;
        eqns_.limit(x1);
        fx = eqns_.fX(x1, li);
    }
    return a;
}


Foam::scalar Foam::minimizationScheme::lineSearch
(
    const scalarList& x0,
    const scalarList& grad,
    const label li,
    scalar& fx
) const
{
    if (debug > 3)
    {
        Info<< "Conducting line search" << endl;
    }
    scalar alpha = 10.0;

    const scalar fx0(fx);
    scalarField x1(x0 - alpha*grad);
    eqns_.limit(x1);
    fx = eqns_.fX(x1, li);

    label iter = 0;
    while (fx >= fx0 && iter++ < maxSteps_)
    {
        alpha *= tau_;
        x1 = x0 - alpha*grad;
        eqns_.limit(x1);
        fx = eqns_.fX(x1, li);
    }
    return alpha;
}


Foam::scalar Foam::minimizationScheme::norm(const scalarList& lst) const
{
    scalar sum = 0;
    forAll(lst, i)
    {
        sum += magSqr(lst[i]);
    }
    return sqrt(sum/scalar(lst.size()));
}


Foam::scalar Foam::minimizationScheme::inner
(
    const scalarList& lst1,
    const scalarList& lst2
) const
{
    scalar sum = 0;
    forAll(lst1, i)
    {
        sum += lst1[i]*lst2[i];
    }
    return sum;
}


void Foam::minimizationScheme::sample
(
    scalarList& xLow,
    scalarList& xHigh,
    labelList& xBest,
    scalar& yBest,
    labelList& indicies,
    const label li,
    const label diri
) const
{
    if ((debug) && diri == 0)
    {
        Info<<"Pre sampling interval" << endl;
    }

    if (diri >= xBest.size())
    {
        scalar y = eqns_.fX((xLow + xHigh)*0.5, li);

        if (y < yBest)
        {
            yBest = y;
            xBest = indicies;
        }
        return;
    }


    scalar dx = (xHigh[diri] - xLow[diri])/scalar(nSamples_[diri] + 1);
    for (label i = 0; i < nSamples_[diri]; i++)
    {
        scalarField x0(xLow);
        x0[diri] += dx*scalar(i);
        scalarField x1(xHigh);
        x1[diri] += dx;
        indicies[diri] = i;
        sample(x0, x1, xBest, yBest, indicies, li, diri + 1);
    }

    if (diri != 0)
    {
        return;
    }

    forAll(xLow, i)
    {
        scalar dx = (xHigh[i] - xLow[i])/scalar(nSamples_[i] + 1);
        xLow[i] += scalar(xBest[i])*dx;
        xHigh[i] = xLow[i] + dx;
    }

    if (debug)
    {
        Info<< "Found minimum values in (" << xLow << "," << xHigh << ")"
            << " yBest: " << yBest << endl;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::minimizationScheme::minimizationScheme
(
    const scalarUnivariateEquation& eqns,
    const dictionary& dict
)
:
    eqns_(eqns),
    xTolerances_
    (
        dict.lookupOrDefaultBackwardsCompatible
        (
            {"xTolerances", "tolerances"},
            scalarList
            (
                eqns_.nVar(),
                dict.lookupOrDefaultBackwardsCompatible
                (
                    {"xTolerance", "tolerance"},
                    1e-6
                )
            )
        )
    ),
    xRelTolerances_
    (
        dict.lookupOrDefaultBackwardsCompatible
        (
            {"xRelTolerances", "tolerances"},
            scalarList
            (
                eqns_.nVar(),
                dict.lookupOrDefaultBackwardsCompatible
                (
                    {"xRelTolerance", "tolerance"},
                    1e-6
                )
            )
        )
    ),
    yTolerances_
    (
        dict.lookupOrDefaultBackwardsCompatible
        (
            {"yTolerances", "tolerances"},
            scalarList
            (
                eqns_.nVar(),
                dict.lookupOrDefaultBackwardsCompatible
                (
                    {"yTolerance", "tolerance"},
                    1e-6
                )
            )
        )
    ),
    yRelTolerances_
    (
        dict.lookupOrDefaultBackwardsCompatible
        (
            {"yRelTolerances", "tolerances"},
            scalarList
            (
                eqns_.nVar(),
                dict.lookupOrDefaultBackwardsCompatible
                (
                    {"yRelTolerance", "tolerance"},
                    1e-6
                )
            )
        )
    ),
    maxSteps_(dict.lookupOrDefault<scalar>("maxSteps", 100)),
    stepi_(0),
    xErrors_(eqns.nVar(), great),
    xRelErrors_(eqns.nVar(), great),
    yErrors_(eqns.nVar(), great),
    yRelErrors_(eqns.nVar(), great),
    nSamples_
    (
        dict.lookupOrDefault<labelList>
        (
            "nSamples",
            labelList
            (
                eqns.nVar(),
                dict.lookupOrDefault("nSample", 0)
            )
        )
    ),
    normalize_(dict.lookupOrDefault("normaliseError", true)),
    tau_(dict.lookupOrDefault<scalar>("tau", 0.5)),
    checkY_(false)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::scalarField> Foam::minimizationScheme::solve() const
{
    return solve
    (
        (eqns_.lowerLimits() + eqns_.upperLimits())*0.5,
        eqns_.lowerLimits(),
        eqns_.upperLimits(),
        0
    );
}


Foam::tmp<Foam::scalarField> Foam::minimizationScheme::solve
(
    const scalarList& x0
) const
{
    return solve(x0, eqns_.lowerLimits(), eqns_.upperLimits(), 0);
}


Foam::tmp<Foam::scalarField> Foam::minimizationScheme::solve
(
    const scalarList& x0,
    const label li
) const
{
    return this->solve(x0, eqns_.lowerLimits(), eqns_.upperLimits(), li);
}


Foam::tmp<Foam::scalarField> Foam::minimizationScheme::solve
(
    const scalarList& x0,
    const scalarList& xLow,
    const scalarList& xHigh
) const
{
    return solve(x0, xLow, xHigh, 0);
}


Foam::tmp<Foam::scalarField> Foam::minimizationScheme::solve
(
    const scalarList& x0,
    const scalarList& xLow,
    const scalarList& xHigh,
    const label li
) const
{
    scalarList x1(xLow);
    scalarList x2(xHigh);
    scalarList xStart(x0);
    if (min(nSamples_) > 0)
    {
        labelList xBest(x1.size(), 0);
        labelList indicies(xBest);
        scalar yBest = great;
        sample(x1, x2, xBest, yBest, indicies, li, 0);
        xStart = 0.5*(x1 + x2);
    }
    return minimize(xStart, x1, x2, li);
}

// ************************************************************************* //
