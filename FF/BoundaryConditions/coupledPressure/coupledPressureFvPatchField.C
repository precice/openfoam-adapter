#include "coupledPressureFvPatchField.H"
#include "addToRunTimeSelectionTable.H"
#include "surfaceFields.H"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const fvPatch& p,
    const DimensionedField<Foam::scalar, volMesh>& iF)
: fixedFluxExtrapolatedPressureFvPatchScalarField(p, iF),
  refValue_(p.size(), Zero),
  refGrad_(p.size(), Zero),
  valueFraction_(p.size(), Zero)
{
}


Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const fvPatch& p,
    const DimensionedField<Foam::scalar, volMesh>& iF,
    const dictionary& dict,
    const bool valueRequired)
: fixedFluxExtrapolatedPressureFvPatchScalarField(p, iF, dict),
  refValue_("refValue", dict, p.size()),
  valueFraction_(p.size(), Zero),
  phiName_(dict.getOrDefault<word>("phi", "phi")),
  uName_(dict.getOrDefault<word>("U", "U"))
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
    const fvPatchFieldMapper& mapper)
: fixedFluxExtrapolatedPressureFvPatchScalarField(ptf, p, iF, mapper),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{
    if (notNull(iF) && mapper.hasUnmapped())
    {
        WarningInFunction
            << "On field " << iF.name() << " patch " << p.name()
            << " patchField " << this->type()
            << " : mapper does not map all values." << nl
            << "    To avoid this warning fully specify the mapping in derived"
            << " patch fields." << endl;
    }
}


Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const coupledPressureFvPatchField& ptf)
: fixedFluxExtrapolatedPressureFvPatchScalarField(ptf),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{
}


Foam::coupledPressureFvPatchField::coupledPressureFvPatchField(
    const coupledPressureFvPatchField& ptf,
    const DimensionedField<Foam::scalar, volMesh>& iF)
: fixedFluxExtrapolatedPressureFvPatchScalarField(ptf, iF),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{
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
    const vectorField n = this->patch().nf();

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
    this->writeEntry("value", os);
    this->valueFraction().writeEntry("valueFraction", os);
    this->refValue().writeEntry("refValue", os);
}


// ************************************************************************* //

namespace Foam
{
makePatchTypeField(
    fixedFluxExtrapolatedPressureFvPatchScalarField,
    coupledPressureFvPatchField);
}
