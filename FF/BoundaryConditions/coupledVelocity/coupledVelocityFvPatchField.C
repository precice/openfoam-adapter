/*---------------------------------------------------------------------------*\
    Copyright (C) 2017  Gerasimos Chourdakis
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
* Copyright (C) 2025 Gesellschaft fuer Anlagen- und Reaktorsicherheit         *
*                         (GRS) gGmbH                                         *
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM-preCICE adapter.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version with terms added by GRS.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License with terms by GRS for more details.

    You should have received a copy of the GNU General Public License
    with terms by GRS along with this program. If not, please
    contact your conveyor or GRS gGmbH.
    For a copy of the unmodified GNU General Public License, see
    <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "coupledVelocityFvPatchField.H"
#include "dictionary.H"
#include "addToRunTimeSelectionTable.H"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


Foam::coupledVelocityFvPatchField::coupledVelocityFvPatchField(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF)
: fvPatchField<vector>(p, iF),
  refValue_(p.size(), Zero),
  refGrad_(p.size(), Zero),
  valueFraction_(p.size(), Zero)
{
}


Foam::coupledVelocityFvPatchField::coupledVelocityFvPatchField(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict)
: fvPatchField<vector>(p, iF),
  refValue_("refValue", dict, p.size()),
  valueFraction_(p.size(), Zero),
  phiName_(static_cast<word>(dict.lookup("phi")))
{
    if (dict.found("refGradient"))
    {
        this->refGrad() = vectorField("refGradient", dict, p.size());
    }
    else
    {
        this->refGrad() = vectorField(p.size(), Zero);
    }

    vectorField::operator=(
        valueFraction_* refValue_
        + (1.0 - valueFraction_)
              * (this->patchInternalField()
                 + refGrad_ / this->patch().deltaCoeffs()));
}


Foam::coupledVelocityFvPatchField::coupledVelocityFvPatchField(
    const coupledVelocityFvPatchField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper)
: fvPatchField<vector>(ptf, p, iF, mapper)
{
    this->refValue() = ptf.refValue_;
    this->refGrad() = ptf.refGrad_;
    this->valueFraction() = ptf.valueFraction_;
}


Foam::coupledVelocityFvPatchField::coupledVelocityFvPatchField(
    const coupledVelocityFvPatchField& ptf,
    const DimensionedField<vector, volMesh>& iF)
: fvPatchField<vector>(ptf, iF)
{
    this->refValue() = ptf.refValue_;
    this->refGrad() = ptf.refGrad_;
    this->valueFraction() = ptf.valueFraction_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<vectorField> Foam::coupledVelocityFvPatchField::snGrad() const
{
    return valueFraction_
             * (refValue_ - this->patchInternalField())
             * this->patch().deltaCoeffs()
         + (1.0 - valueFraction_) * refGrad_;
}

void Foam::coupledVelocityFvPatchField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }
    const Foam::surfaceScalarField* phi = &db().lookupObject<surfaceScalarField>(phiName_);
    const scalarField& phip = phi->boundaryField()[this->patch().index()];
    const vectorField n(this->patch().nf());

    int t0 = this->patch().boundaryMesh().mesh().time().startTimeIndex();
    int t = this->patch().boundaryMesh().mesh().time().timeIndex();
    if (t - t0 == 1)
    {
        this->valueFraction() = 1 - pos0(refValue_ & n);
    }
    else
    {
        this->valueFraction() = 1 - pos0(phip);
    }
    fvPatchVectorField::updateCoeffs();
}

void Foam::coupledVelocityFvPatchField::evaluate(const Pstream::commsTypes p)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    vectorField::operator=(
        valueFraction_* refValue_
        + (1.0 - valueFraction_)
              * (this->patchInternalField()
                 + refGrad_ / this->patch().deltaCoeffs()));

    fvPatchVectorField::evaluate();
}


Foam::tmp<Foam::Field<vector>>
Foam::coupledVelocityFvPatchField::valueInternalCoeffs(
    const tmp<scalarField>&) const
{
    return (pTraits<Foam::vector>::one) * (1.0 - valueFraction_);
}


Foam::tmp<Foam::Field<vector>>
Foam::coupledVelocityFvPatchField::valueBoundaryCoeffs(
    const tmp<scalarField>&) const
{
    return valueFraction_ * refValue_
         + (1.0 - valueFraction_) * refGrad_ / this->patch().deltaCoeffs();
}


Foam::tmp<Foam::Field<vector>>
Foam::coupledVelocityFvPatchField::gradientInternalCoeffs() const
{
    return -(pTraits<Foam::vector>::one) * valueFraction_ * this->patch().deltaCoeffs();
}


Foam::tmp<Foam::Field<vector>>
Foam::coupledVelocityFvPatchField::gradientBoundaryCoeffs() const
{
    return valueFraction_ * this->patch().deltaCoeffs() * refValue_
         + (1.0 - valueFraction_) * refGrad_;
}


void Foam::coupledVelocityFvPatchField::write(Ostream& os) const
{
    fvPatchField<vector>::write(os);
    writeEntry(os, "value", *this);
    writeEntry(os, "valueFraction", this->valueFraction());
    writeEntry(os, "refValue", this->refValue());
}


// ************************************************************************* //

namespace Foam
{
makePatchTypeField(
    fvPatchVectorField,
    coupledVelocityFvPatchField);
}
