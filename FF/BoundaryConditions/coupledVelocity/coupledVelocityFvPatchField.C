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
  phiName_(dict.getOrDefault<word>("phi", "phi"))
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
: fvPatchField<vector>(ptf, p, iF, mapper),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{
}


Foam::coupledVelocityFvPatchField::coupledVelocityFvPatchField(
    const coupledVelocityFvPatchField& ptf)
: fvPatchField<vector>(ptf),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{
}


Foam::coupledVelocityFvPatchField::coupledVelocityFvPatchField(
    const coupledVelocityFvPatchField& ptf,
    const DimensionedField<vector, volMesh>& iF)
: fvPatchField<vector>(ptf, iF),
  refValue_(ptf.refValue_),
  refGrad_(ptf.refGrad_),
  valueFraction_(ptf.valueFraction_)
{
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
    const vectorField n = this->patch().nf();

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
    this->writeEntry("value", os);
    this->valueFraction().writeEntry("valueFraction", os);
    this->refValue().writeEntry("refValue", os);
}


// ************************************************************************* //

namespace Foam
{
makePatchTypeField(
    fvPatchVectorField,
    coupledVelocityFvPatchField);
}
