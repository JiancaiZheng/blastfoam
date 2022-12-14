/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
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

\*---------------------------------------------------------------------------*/

#include "burstCyclicFvPatchField.H"
#include "extrapolatedCalculatedFvPatchField.H"
#include "volFields.H"
#include "burstCyclicFvPatch.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::burstCyclicFvPatchField<Type>::burstCyclicFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    cyclicFvPatchField<Type>(p, iF),
    burstFvPatchFieldBase(p),
    intactPatchField_
    (
        new calculatedFvPatchField<Type>
        (
            p,
            iF
        )
    )
{
    this->operator=(pTraits<Type>::zero);
}


template<class Type>
Foam::burstCyclicFvPatchField<Type>::burstCyclicFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    cyclicFvPatchField<Type>(p, iF, dict),
    burstFvPatchFieldBase(p),
    intactPatchField_()
{
    if (!isA<burstCyclicFvPatch>(p))
    {
        FatalIOErrorInFunction
        (
            dict
        )   << "    patch type '" << p.type()
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->internalField().name()
            << " in file " << this->internalField().objectPath()
            << exit(FatalIOError);
    }

    // Create a new patch dictionary and replace the type with the intactType
    dictionary intactDict(dict.parent(), dict.subDict("intactPatch"));
    if (!intactDict.found("patchType"))
    {
        intactDict.add("patchType", typeName);
    }

    intactPatchField_ =
        fvPatchField<Type>::New
        (
            p,
            iF,
            intactDict
        );

    cyclicFvPatchField<Type>::evaluate(Pstream::commsTypes::blocking);
}


template<class Type>
Foam::burstCyclicFvPatchField<Type>::burstCyclicFvPatchField
(
    const burstCyclicFvPatchField<Type>& bpf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    cyclicFvPatchField<Type>(bpf, p, iF, mapper),
    burstFvPatchFieldBase(p),
    intactPatchField_
    (
        fvPatchField<Type>::New
        (
            bpf.intactPatchField_(),
            p,
            iF,
            mapper
        )
    )
{}


template<class Type>
Foam::burstCyclicFvPatchField<Type>::burstCyclicFvPatchField
(
    const burstCyclicFvPatchField<Type>& bpf,
    const DimensionedField<Type, volMesh>& iF
)
:
    cyclicFvPatchField<Type>
    (
        static_cast<const  cyclicFvPatchField<Type>&>(bpf),
        iF
    ),
    burstFvPatchFieldBase(this->patch()),
    intactPatchField_(bpf.intactPatchField_->clone(iF))
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::burstCyclicFvPatchField<Type>::patchNeighbourField() const
{
    if (this->unblock_)
    {
        return cyclicFvPatchField<Type>::patchNeighbourField();
    }
    else if (this->block_)
    {
        return
            intactPatchField_->coupled()
          ? intactPatchField_().patchNeighbourField()
          : intactPatchField_();
    }
    return
        intact()
       *(
            intactPatchField_->coupled()
          ? intactPatchField_().patchNeighbourField()
          : intactPatchField_()
        )
      + (1.0 - intact())*cyclicFvPatchField<Type>::patchNeighbourField();
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::autoMap
(
    const fvPatchFieldMapper& m
)
{
    cyclicFvPatchField<Type>::autoMap(m);
    intactPatchField_->autoMap(m);
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::rmap
(
    const fvPatchField<Type>& ptf,
    const labelList& addr
)
{
    cyclicFvPatchField<Type>::rmap(ptf, addr);

    const burstCyclicFvPatchField<Type>& bpf =
        refCast<const burstCyclicFvPatchField<Type>>(ptf);
    intactPatchField_->rmap(bpf.intactPatchField_(), addr);
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    // Set the intact field to zero gradient in the event that the
    // patchField has not been updated
    intactPatchField_.ref() = this->patchInternalField();

    intactPatchField_->updateCoeffs();
    cyclicFvPatchField<Type>::updateCoeffs();
    burstFvPatchFieldBase::update();
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::initEvaluate
(
    const Pstream::commsTypes commsType
)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    cyclicFvPatchField<Type>::initEvaluate(commsType);
    intactPatchField_->initEvaluate(commsType);
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::evaluate
(
    const Pstream::commsTypes commsType
)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    intactPatchField_->evaluate(commsType);
    if (block_)
    {
        Field<Type>::operator=(intactPatchField_());
    }
    else
    {
        cyclicFvPatchField<Type>::evaluate(commsType);
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::burstCyclicFvPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalarField>& w
) const
{
    return
        intactPatchField_().valueInternalCoeffs(w)*intact()
      + cyclicFvPatchField<Type>::valueInternalCoeffs(w)
       *(1.0 - intact());
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::burstCyclicFvPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>& w
) const
{
    return
        intactPatchField_().valueBoundaryCoeffs(w)*intact()
      + cyclicFvPatchField<Type>::valueBoundaryCoeffs(w)
       *(1.0 - intact());
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::burstCyclicFvPatchField<Type>::gradientInternalCoeffs
(
    const scalarField& deltaCoeffs
) const
{
    return
        (
            intactPatchField_().coupled()
          ? intactPatchField_().gradientInternalCoeffs(deltaCoeffs)
          : intactPatchField_().gradientInternalCoeffs()
        )*intact()
      + cyclicFvPatchField<Type>::gradientInternalCoeffs(deltaCoeffs)
       *(1.0 - intact());
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::burstCyclicFvPatchField<Type>::gradientBoundaryCoeffs
(
    const scalarField& deltaCoeffs
) const
{
    return
        (
            intactPatchField_().coupled()
          ? intactPatchField_().gradientBoundaryCoeffs(deltaCoeffs)
          : intactPatchField_().gradientBoundaryCoeffs()
        )*intact()
      + cyclicFvPatchField<Type>::gradientBoundaryCoeffs(deltaCoeffs)
       *(1.0 - intact());
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::updateInterfaceMatrix
(
    scalarField& result,
    const scalarField& psiInternal,
    const scalarField& coeffs,
    const direction cmpt,
    const Pstream::commsTypes commsType
) const
{
    const scalarField intact(this->intact());
    const polyPatch& p = this->patch().patch();
    {
        scalarField cResult(result.size(), Zero);
        cyclicFvPatchField<Type>::updateInterfaceMatrix
        (
            cResult,
            psiInternal,
            coeffs,
            cmpt,
            commsType
        );
        forAll(p.faceCells(), fi)
        {
            const label celli = p.faceCells()[fi];
            result[celli] += (1.0 - intact[fi])*cResult[celli];
        }
    }

    if (isA<coupledFvPatchField<Type>>(intactPatchField_()))
    {
        scalarField iResult(result.size(), Zero);
        refCast<const coupledFvPatchField<Type>>
        (
            intactPatchField_()
        ).updateInterfaceMatrix
        (
            iResult,
            psiInternal,
            coeffs,
            cmpt,
            commsType
        );
        forAll(p.faceCells(), fi)
        {
            const label celli = p.faceCells()[fi];
            result[celli] += intact[fi]*iResult[celli];
        }
    }
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::updateInterfaceMatrix
(
    Field<Type>& result,
    const Field<Type>& psiInternal,
    const scalarField& coeffs,
    const Pstream::commsTypes commsType
) const
{
    const polyPatch& p = this->patch().patch();
    const scalarField intact(this->intact());
    {
        Field<Type> cResult(result.size(), Zero);
        cyclicFvPatchField<Type>::updateInterfaceMatrix
        (
            cResult,
            psiInternal,
            coeffs,
            commsType
        );
        forAll(p.faceCells(), fi)
        {
            const label celli = p.faceCells()[fi];
            result[celli] += (1.0 - intact[fi])*cResult[celli];
        }
    }
    if (isA<coupledFvPatchField<Type>>(intactPatchField_()))
    {
        Field<Type> iResult(result.size(), Zero);
        refCast<const coupledFvPatchField<Type>>
        (
            intactPatchField_()
        ).updateInterfaceMatrix
        (
            iResult,
            psiInternal,
            coeffs,
            commsType
        );
        forAll(p.faceCells(), fi)
        {
            const label celli = p.faceCells()[fi];
            result[celli] += intact[fi]*iResult[celli];
        }
    }
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::write(Ostream& os) const
{
    cyclicFvPatchField<Type>::write(os);

    writeKeyword(os, "intactPatch")
        << nl << indent << token::BEGIN_BLOCK << nl << incrIndent;
    intactPatchField_->write(os);
    os << decrIndent << indent << token::END_BLOCK << endl;
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator=
(
    const UList<Type>& ul
)
{
    Field<Type>::operator=(ul);
    intactPatchField_.ref() = ul;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator=
(
    const fvPatchField<Type>& ptf
)
{
    Field<Type>::operator=(ptf);
    intactPatchField_.ref() = ptf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator+=
(
    const fvPatchField<Type>& ptf
)
{
    Field<Type>::operator+=(ptf);
    intactPatchField_.ref() += ptf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator-=
(
    const fvPatchField<Type>& ptf
)
{
    Field<Type>::operator-=(ptf);
    intactPatchField_.ref() -= ptf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator*=
(
    const fvPatchField<scalar>& ptf
)
{
    Field<Type>::operator*=(ptf);
    intactPatchField_.ref() *= ptf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator/=
(
    const fvPatchField<scalar>& ptf
)
{
    Field<Type>::operator/=(ptf);
    intactPatchField_.ref() /= ptf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator+=
(
    const Field<Type>& tf
)
{
    Field<Type>::operator+=(tf);
    intactPatchField_.ref() += tf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator-=
(
    const Field<Type>& tf
)
{
    Field<Type>::operator-=(tf);
    intactPatchField_.ref() -= tf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator*=
(
    const scalarField& tf
)
{
    Field<Type>::operator*=(tf);
    intactPatchField_.ref() *= tf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator/=
(
    const scalarField& tf
)
{
    Field<Type>::operator/=(tf);
    intactPatchField_.ref() /= tf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator=
(
    const Type& t
)
{
    Field<Type>::operator=(t);
    intactPatchField_.ref() = t;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator+=
(
    const Type& t
)
{
    Field<Type>::operator+=(t);
    intactPatchField_.ref() += t;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator-=
(
    const Type& t
)
{
    Field<Type>::operator-=(t);
    intactPatchField_.ref() -= t;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator*=
(
    const scalar s
)
{
    Field<Type>::operator*=(s);
    intactPatchField_.ref() *= s;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator/=
(
    const scalar s
)
{
    Field<Type>::operator/=(s);
    intactPatchField_.ref() /= s;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator==
(
    const fvPatchField<Type>& ptf
)
{
    Field<Type>::operator=(ptf);
    intactPatchField_.ref() == ptf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator==
(
    const Field<Type>& tf
)
{
    Field<Type>::operator=(tf);
    intactPatchField_.ref() == tf;
}


template<class Type>
void Foam::burstCyclicFvPatchField<Type>::operator==
(
    const Type& t
)
{
    Field<Type>::operator=(t);
    intactPatchField_.ref() == t;
}

// ************************************************************************* //
