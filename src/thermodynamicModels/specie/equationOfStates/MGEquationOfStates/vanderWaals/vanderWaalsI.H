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

\*---------------------------------------------------------------------------*/

#include "constants.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class Specie>
inline Foam::vanderWaals<Specie>::vanderWaals
(
    const Specie& sp,
    const scalar a,
    const scalar b,
    const scalar c,
    const scalar gamma
)
:
    Specie(sp),
    a_(a),
    b_(b),
    c_(c),
    gamma_(gamma)
{}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::Pi
(
    const scalar rho,
    const scalar e
) const
{
    return
        (1.0 - (gamma_ - 1.0)/stabilise((1.0 - b_*rho), 1e-10))*a_*sqr(rho)
      + ((gamma_ - 1.0)/stabilise((1.0 - b_*rho), 1e-10) + 1.0)*c_;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::delta
(
    const scalar p,
    const scalar rho,
    const scalar e
) const
{
    scalar pi
    (
        (gamma_ - 1.0)/stabilise((1.0 - b_*rho), 1e-10)
       *(rho*e + a_*sqr(rho) - c_)
      - (a_*sqr(rho) + c_)
    );
    return
        -b_*(pi + a_*sqr(rho))/(gamma_ - 1.0)
       + ((1.0 - b_*rho)/(gamma_ - 1.0) - 1.0)*2.0*a_*rho;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::GammaMG
(
    const scalar rho,
    const scalar e
) const
{
    return (gamma_ - 1.0)/stabilise((1.0 - b_*rho), 1e-10) + 1.0;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Specie>
inline Foam::vanderWaals<Specie>::vanderWaals
(
    const word& name,
    const vanderWaals<Specie>& pf
)
:
    Specie(name, pf),
    a_(pf.a_),
    b_(pf.b_),
    c_(pf.c_),
    gamma_(pf.gamma_)
{}


template<class Specie>
inline Foam::autoPtr<Foam::vanderWaals<Specie>>
Foam::vanderWaals<Specie>::clone() const
{
    return autoPtr<vanderWaals<Specie>>
    (
        new vanderWaals<Specie>(*this)
    );
}


template<class Specie>
inline Foam::autoPtr<Foam::vanderWaals<Specie>>
Foam::vanderWaals<Specie>::New
(
    const dictionary& dict
)
{
    return autoPtr<vanderWaals<Specie>>
    (
        new vanderWaals<Specie>(dict)
    );
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::p
(
    const scalar rho,
    const scalar e,
    const scalar T,
    const bool limit
) const
{
    return
        limit
      ? max((GammaMG(rho, e) - 1.0)*rho*e - Pi(rho, e), 0.0)
      : (GammaMG(rho, e) - 1.0)*rho*e - Pi(rho, e);
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::rho
(
    const scalar p,
    const scalar T
) const
{
    NotImplemented;
    return 0;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::Gamma
(
    const scalar rho,
    const scalar e,
    const scalar T,
    const scalar Cv
) const
{
    return GammaMG(rho, e);
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::cSqr
(
    const scalar p,
    const scalar rho,
    const scalar e,
    const scalar T,
    const scalar Cv
) const
{
    scalar G(GammaMG(rho, e));
    scalar h((G*p + Pi(rho, e))/((G - 1.0)*max(rho, 1e-10)));
    return (h - delta(p, rho, e))*(G - 1.0);
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::dpdv
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    return
        sqr(rho)/sqr(1.0 - b_*rho)
       *(
            2.0*a_*rho*(b_*rho - 1.0)*(b_*rho + gamma_ - 2.0)
          + b_*c_*(gamma_ - 1.0)
          - a_*b_*sqr(rho)*(gamma_ - 1.0)
          - (gamma_ - 1.0)*e
        );
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::dpde
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    return (GammaMG(rho, e) - 1.0)*rho;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::dpdT
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    NotImplemented;
    return 0.0;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::Cp
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    return 0.0;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::Cv
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    return 0.0;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::CpMCv
(
    const scalar rho,
    const scalar e,
    const scalar T,
    const scalar CpCv
) const
{
    scalar G(this->GammaMG(rho, e));
    return this->eBased_ ? (G - 1.0)*CpCv : CpCv*(G - 1.0)/G;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::E
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    scalar rhos(max(rho, 1e-10));
    return
      - a_*rho
      + c_*gamma_/rhos
      - (gamma_ - 1.0)/b_
       *(
            a_*log(rhos)
          - (a_ - sqr(b_)*c_)*log(rhos/stabilise((1.0 - b_*rho), 1e-10))
        );
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::H
(
    const scalar rho,
    const scalar e,
    const scalar T
) const
{
    return
        rho/stabilise((1.0 - b_*rho), 1e-10)
       *(
            (gamma_ - 1.0)*e
          + 2.0*a_*b_*rho
          + a_*(gamma_ - 3.0)
          - sqr(b_)*c_*(gamma_ - 1.0)
        )
      - (gamma_ - 1.0)
       *(
            a_*log(rho)
          - (a_ - sqr(b_)*c_)*log(rho/stabilise((1.0 - b_*rho), 1e-10))
        )/b_;
}


template<class Specie>
Foam::scalar Foam::vanderWaals<Specie>::S
(
    const scalar p,
    const scalar T
) const
{
    NotImplemented;
    return 0;
}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Specie>
inline void Foam::vanderWaals<Specie>::operator+=
(
    const vanderWaals<Specie>& pf
)
{
    scalar Y1 = this->Y();
    Specie::operator+=(pf);

    if (mag(this->Y()) > SMALL)
    {
        Y1 /= this->Y();
        const scalar Y2 = pf.Y()/this->Y();

        a_ = Y1*a_ + Y2*pf.a_;
        b_ = Y1*b_ + Y2*pf.b_;
        c_ = Y1*c_ + Y2*pf.c_;
        gamma_ = Y1*gamma_ + Y2*pf.gamma_;
    }
}


template<class Specie>
inline void Foam::vanderWaals<Specie>::operator*=(const scalar s)
{
    Specie::operator*=(s);
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

template<class Specie>
inline Foam::vanderWaals<Specie> Foam::operator+
(
    const vanderWaals<Specie>& pf1,
    const vanderWaals<Specie>& pf2
)
{
    Specie sp
    (
        static_cast<const Specie&>(pf1)
      + static_cast<const Specie&>(pf2)
    );

    if (mag(sp.Y()) < SMALL)
    {
        return vanderWaals<Specie>
        (
            sp,
            pf1.a_,
            pf1.b_,
            pf1.c_,
            pf1.gamma_
        );
    }
    else
    {
        const scalar Y1 = pf1.Y()/sp.Y();
        const scalar Y2 = pf2.Y()/sp.Y();

        return vanderWaals<Specie>
        (
            sp,
            Y1*pf1.a_ + Y2*pf2.a_,
            Y1*pf1.b_ + Y2*pf2.b_,
            Y1*pf1.c_ + Y2*pf2.c_,
            Y1*pf1.gamma_ + Y2*pf2.gamma_
        );
    }
}


template<class Specie>
inline Foam::vanderWaals<Specie> Foam::operator*
(
    const scalar s,
    const vanderWaals<Specie>& pf
)
{
    return vanderWaals<Specie>
    (
        s*static_cast<const Specie&>(pf),
        pf.a_,
        pf.b_,
        pf.c_,
        pf.gamma_
    );
}


template<class Specie>
inline Foam::vanderWaals<Specie> Foam::operator==
(
    const vanderWaals<Specie>& pf1,
    const vanderWaals<Specie>& pf2
)
{
    Specie sp
    (
        static_cast<const Specie&>(pf1)
     == static_cast<const Specie&>(pf2)
    );

    const scalar Y1 = pf1.Y()/sp.Y();
    const scalar Y2 = pf2.Y()/sp.Y();

    return vanderWaals<Specie>
    (
        sp,
        Y2*pf2.a_    - Y1*pf1.a_,
        Y2*pf2.b_  - Y1*pf1.b_,
        Y2*pf2.c_ - Y1*pf1.c_,
        Y2*pf2.gamma_     - Y1*pf1.gamma_
    );
}


// ************************************************************************* //
