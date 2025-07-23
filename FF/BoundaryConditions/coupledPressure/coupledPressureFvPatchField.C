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

#include "coupledPressureFvPatchField.H"
#include "addToRunTimeSelectionTable.H"
#include "surfaceFields.H"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const fvPatch& p,
    const DimensionedField<Foam::scalar, volMesh>& iF,
    const dictionary& dict,
    const bool valueRequired)
: fixedFluxExtrapolatedPressureFvPatchScalarField(p, iF, dict),
  refValue_("refValue", dict, p.size()),
  valueFraction_(p.size(), Zero),
  phiName_(static_cast<word>(dict.lookup("phi"))),
  uName_(static_cast<word>(dict.lookup("U")))
{
    if (dict.found("refGradient"))
    {
        this->refGrad() = scalarField("refGradient", dict, p.size());
    }
    else
    {
        this->refGrad() = scalarField(p.size(), Zero);
    }
}


Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const coupledPressureFvPatchField& ptf,
    const fvPatch& p,
    const DimensionedField<Foam::scalar, volMesh>& iF,
    const fieldMapper& mapper)
: fixedFluxExtrapolatedPressureFvPatchScalarField(ptf, p, iF, mapper),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{}

Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const coupledPressureFvPatchField& ptf,
    const DimensionedField<Foam::scalar, volMesh>& iF)
: fixedFluxExtrapolatedPressureFvPatchScalarField(ptf, iF)
{
    this->refValue() = ptf.refValue_;
    this->refGrad() = ptf.refGrad_;
    this->valueFraction() = ptf.valueFraction_;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


Foam::tmp<Foam::scalarField>
Foam::coupledPressureFvPatchField::snGrad() const
{
    return valueFraction_
             * (refValue_ - this->patchInternalField())
             * this->patch().deltaCoeffs()
         + (1.0 - valueFraction_) * refGrad_;
}

void Foam::coupledPressureFvPatchField::updateCoeffs()
{
    const Foam::surfaceScalarField* phi = &db().lookupObject<surfaceScalarField>(phiName_);
    const scalarField& phip = phi->boundaryField()[this->patch().index()];
    const Foam::volVectorField* U = &db().lookupObject<volVectorField>(uName_);
    const vectorField& Up = U->boundaryField()[this->patch().index()];
    const vectorField n(this->patch().nf());

    int t0 = this->patch().boundaryMesh().mesh().time().startTimeIndex();
    int t = this->patch().boundaryMesh().mesh().time().timeIndex();
    if (t - t0 == 1)
    {
        this->valueFraction() = pos0(Up & n);
    }
    else
    {
        this->valueFraction() = pos0(phip);
    }
    fixedFluxExtrapolatedPressureFvPatchScalarField::updateCoeffs();
}

void Foam::coupledPressureFvPatchField::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    scalarField::operator=(
        valueFraction_* refValue_
        + (1.0 - valueFraction_)
              * (this->patchInternalField()
                 + refGrad_ / this->patch().deltaCoeffs()));

    fvPatchScalarField::evaluate();
}

Foam::tmp<Foam::scalarField>
Foam::coupledPressureFvPatchField::valueInternalCoeffs(
    const tmp<scalarField>&) const
{
    return (pTraits<Foam::scalar>::one) * (1.0 - valueFraction_);
}


Foam::tmp<Foam::scalarField>
Foam::coupledPressureFvPatchField::valueBoundaryCoeffs(
    const tmp<scalarField>&) const
{
    return valueFraction_ * refValue_
         + (1.0 - valueFraction_) * refGrad_ / this->patch().deltaCoeffs();
}


Foam::tmp<Foam::scalarField>
Foam::coupledPressureFvPatchField::gradientInternalCoeffs() const
{
    return -(pTraits<Foam::scalar>::one) * valueFraction_ * this->patch().deltaCoeffs();
}


Foam::tmp<Foam::scalarField>
Foam::coupledPressureFvPatchField::gradientBoundaryCoeffs() const
{
    return valueFraction_ * this->patch().deltaCoeffs() * refValue_
         + (1.0 - valueFraction_) * refGrad_;
}


void Foam::coupledPressureFvPatchField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    writeEntry(os, "value", *this);
    writeEntry(os, "valueFraction", this->valueFraction());
    writeEntry(os, "refValue", this->refValue());
}


// ************************************************************************* //

namespace Foam
{
makePatchTypeField(
    fixedFluxExtrapolatedPressureFvPatchScalarField,
    coupledPressureFvPatchField);
}
