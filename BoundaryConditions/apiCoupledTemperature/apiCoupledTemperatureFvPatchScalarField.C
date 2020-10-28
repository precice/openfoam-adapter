
#include "apiCoupledTemperatureFvPatchScalarField.H"

#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "physicoChemicalConstants.H"

#include "autoPtr.H"
#include "Enum.H"
#include "Function1.H"

#include <limits>
#include <algorithm>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

const Foam::Enum
<
    Foam::apiCoupledTemperatureFvPatchScalarField::operationMode
>
Foam::apiCoupledTemperatureFvPatchScalarField::operationModeNames
({
    { operationMode::fixedTemperature, "fixedTemperature" },
    { operationMode::fixedHeatFlux, "fixedHeatFlux" },
    { operationMode::fixedMixedTemperatureHTC, "HTC" },
});


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
	mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase
    (
        patch(),
        "undefined",
        "undefined",
        "undefined-K",
        "undefined-alpha"
    ),
    mode_(fixedTemperature),
    qrName_("undefined-qr"),
    relaxation_(1),
    qrPrevious_(),
    qrRelaxation_(1),
    T_neighbour_(),
    h_neighbour_(),
    heatflux_()
{
    refValue() = 0;
    refGrad() = 0;
    valueFraction() = 1;
}


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const apiCoupledTemperatureFvPatchScalarField& rhs,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
	mixedFvPatchScalarField(rhs, p, iF, mapper),
    temperatureCoupledBase(patch(), rhs),
    mode_(rhs.mode_),
    qrName_(rhs.qrName_),
    relaxation_(rhs.relaxation_),
    qrPrevious_(rhs.qrPrevious_),
    qrRelaxation_(rhs.qrRelaxation_),
    T_neighbour_(rhs.T_neighbour_),
    h_neighbour_(rhs.h_neighbour_),
    heatflux_(rhs.heatflux_)
{
    switch (mode_)
    {
    case fixedHeatFlux:
        heatflux_.resize(mapper.size());
        heatflux_.map(rhs.heatflux_, mapper);
        break;

    case fixedMixedTemperatureHTC:
        T_neighbour_.resize(mapper.size());
        T_neighbour_.map(rhs.T_neighbour_, mapper);

        h_neighbour_.resize(mapper.size());
        h_neighbour_.map(rhs.h_neighbour_, mapper);
        break;
    }
    
    qrPrevious_.resize(mapper.size());
    qrPrevious_.map(rhs.qrPrevious_, mapper);
}


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
	//mixedFvPatchField<scalar>(p, iF, dict),
	mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), dict),
    mode_(operationModeNames.get("mode", dict)),
    qrName_(dict.getOrDefault<word>("qr", "none")),
    relaxation_(dict.getOrDefault<scalar>("relaxation", scalar(1))),
    qrRelaxation_(dict.getOrDefault<scalar>("qrRelaxation", scalar(1)))
{
    // field value
    if (dict.found("value"))
    {
        fvPatchScalarField::operator=(scalarField("value", dict, p.size()));
    }

    // mixed value
    if (dict.found("refValue"))
    {
        refValue() = scalarField("refValue", dict, p.size());
    }
    else
    {
        refValue() = *this;
    }

    if (dict.found("refGradient"))
    {
        refGrad() = scalarField("refGradient", dict, p.size());
    }
    else
    {
        refGrad() = scalar(0);
    }

    if (dict.found("valueFraction"))
    {
        valueFraction() = scalarField("valueFraction", dict, p.size());
    }
    else
    {
        valueFraction() = scalar(1);
    }

    switch (mode_)
    {
    case fixedHeatFlux:
        heatflux_.resize(p.size(), dict.getOrDefault<scalar>("heatFlux", scalar(0)));
        break;

    case fixedMixedTemperatureHTC:
        h_neighbour_.resize(p.size(), scalar(1.0e-6));
        T_neighbour_.resize(p.size(), scalar(0));

        if (dict.found("hNeighbour"))
        {
            scalarField field ("hNeighbour", dict, p.size());
            forAll(field, i)
            {
                h_neighbour_[i] = field[i];
            }
        }

        if (dict.found("TNeighbour"))
        {
            scalarField field ("TNeighbour", dict, p.size());
            forAll(T_neighbour_, i)
            {
                T_neighbour_[i] = field[i];
            }
        } else if (dict.found("refValue"))
        {
            const scalarField& field (refValue());
            forAll(T_neighbour_, i)
            {
                T_neighbour_[i] = field[i];
            }
        } else if (dict.found("value"))
        {
            const scalarField& field (*this);
            forAll(T_neighbour_, i)
            {
                T_neighbour_[i] = field[i];
            }
        }
        break;
    }

    // radiation field
    if (qrName_ != "none" && dict.found("qrPrevious"))
    {
        qrPrevious_ = scalarField("qrPrevious", dict, p.size());
    }
    else
    {
        qrPrevious_.resize(p.size(), scalar(0));
    }
}


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const apiCoupledTemperatureFvPatchScalarField& rhs
)
:
	mixedFvPatchScalarField(rhs),
    temperatureCoupledBase(rhs),
    mode_(rhs.mode_),
    qrName_(rhs.qrName_),
    relaxation_(rhs.relaxation_),
    qrPrevious_(rhs.qrPrevious_),
    qrRelaxation_(rhs.qrRelaxation_),
    T_neighbour_(rhs.T_neighbour_),
    h_neighbour_(rhs.h_neighbour_),
    heatflux_(rhs.heatflux_)
{}


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const apiCoupledTemperatureFvPatchScalarField& rhs,
    const DimensionedField<scalar, volMesh>& iF
)
:
	mixedFvPatchScalarField(rhs, iF),
    temperatureCoupledBase(patch(), rhs),
    mode_(rhs.mode_),
    qrName_(rhs.qrName_),
    relaxation_(rhs.relaxation_),
    qrPrevious_(rhs.qrPrevious_),
    qrRelaxation_(rhs.qrRelaxation_),
    T_neighbour_(rhs.T_neighbour_),
    h_neighbour_(rhs.h_neighbour_),
    heatflux_(rhs.heatflux_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::apiCoupledTemperatureFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& mapper
)
{
    mixedFvPatchScalarField::autoMap(mapper);

    switch (mode_)
    {
    case fixedHeatFlux:
        heatflux_.autoMap(mapper);
        break;
    case fixedMixedTemperatureHTC:
        T_neighbour_.autoMap(mapper);
        h_neighbour_.autoMap(mapper);
        break;
    }

    qrPrevious_.autoMap(mapper);
}


void Foam::apiCoupledTemperatureFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    mixedFvPatchScalarField::rmap(ptf, addr);
    const auto& rhs = refCast<const apiCoupledTemperatureFvPatchScalarField>(ptf);

    switch (mode_)
    {
    case fixedHeatFlux:
        heatflux_.rmap(rhs.heatflux_, addr);
        break;
    case fixedMixedTemperatureHTC:
        T_neighbour_.rmap(rhs.T_neighbour_, addr);
        h_neighbour_.rmap(rhs.h_neighbour_, addr);
        break;
    }


    qrPrevious_.rmap(rhs.qrPrevious_, addr);
}


// https://www.openfoam.com/documentation/guides/latest/api/turbulentTemperatureCoupledBaffleMixedFvPatchScalarField_8C_source.html
//
// Both sides agree on
// - temperature : (myKDelta*fld + nbrKDelta*nbrFld)/(myKDelta+nbrKDelta)
// - gradient    : (temperature-fld)*delta
// We've got a degree of freedom in how to implement this in a mixed bc.
// (what gradient, what fixedValue and mixing coefficient)
// Two reasonable choices:
// 1. specify above temperature on one side (preferentially the high side)
//    and above gradient on the other. So this will switch between pure
//    fixedvalue and pure fixedgradient
// 2. specify gradient and temperature such that the equations are the
//    same on both sides. This leads to the choice of
//    - refGradient = zero gradient
//    - refValue = neighbour value
//    - mixFraction = nbrKDelta / (nbrKDelta + myKDelta())
//

Foam::tmp<Foam::scalarField> Foam::apiCoupledTemperatureFvPatchScalarField::getWallHeatFlux
() const
{
    const scalarField&  Twall (*this);
    auto heatflux (kappa(Twall) * snGrad());

    if (qrName_ != "none")
    {
        auto & flux (heatflux.ref());
        forAll(flux, i)
        {
            flux[i] -= qrPrevious_[i];
        }
    }

    return heatflux;
}

Foam::tmp<Foam::scalarField> Foam::apiCoupledTemperatureFvPatchScalarField::getHeatTransferCoeff
() const
{
    const scalarField&  Twall (*this);
    return kappa(Twall) * patch().deltaCoeffs();
}

const Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::T_Wall
() const
{
    return *this;
}

Foam::tmp<Foam::scalarField> Foam::apiCoupledTemperatureFvPatchScalarField::T_Cell
() const
{
    return patchInternalField();
}

Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::h_Neighbour()
{
    return h_neighbour_;
}
const Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::h_Neighbour
() const
{
    return h_neighbour_;
}

Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::T_Neighbour()
{
    return T_neighbour_;
}
const Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::T_Neighbour
() const
{
    return T_neighbour_;
}

Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::heatFlux()
{
    return heatflux_;
}
const Foam::scalarField& Foam::apiCoupledTemperatureFvPatchScalarField::heatFlux
() const
{
    return heatflux_;
}

//                               T_wall
//                                 |
//  ******************************** *   *   *   *   *   *   *   * 
//  *  domain_1 = cell             *   domain_2                   *
//  *                              *                              
//  *                         q_1  *   q_2                        *
//  *            T_1           <-- * <--         T_2              
//  *             *            <-- * <--          *               *
//  *                          <-- * <--                          
//  *                              *                              *
//  *                 q_radiation  *                              
//  *                          <-- *                              *
//  *                          <-- *                              
//  *                          <-- *                              *
//  *                              *                              
//  *                              *                              *
//  ******************************** *   *   *   *   *   *   *   * 
//                |----- dx_1 -----|---- dx_2 ----|
//
//  q_1 + q_{radiation} = -q2 \approx \frac{kappa_2}{\Delta_2} \cdot \left( T_{wall} - T_2 \right)
//
void Foam::apiCoupledTemperatureFvPatchScalarField::updateCoeffs
()
{
    // stop if up-to-date
    if (updated()) return;

    // qr field
    if (qrName_ != "none")
    {
        const auto data = qrRelaxation_ * patch().lookupPatchField<volScalarField, scalar>(qrName_) + (1 - qrRelaxation_) * qrPrevious_;

        // copy to
        qrPrevious_ = data.cref();
    };

    //
    const scalarField&  Twall   (*this);
    const scalarField&  qr      (qrPrevious_);

    // do update depending on operation mode
    switch (mode_)
    {
    case fixedHeatFlux:
        refGrad() = (heatflux_ + qr) / kappa(Twall);
        refValue() = 0;
        valueFraction() = 0;
        break;

    case fixedMixedTemperatureHTC:
        // get values from mixed-value boundary field
        scalarField &value(refValue());
        scalarField &fract(valueFraction());

        const scalarField refValue0(value);
        const scalarField valueFraction0(fract);
        const scalarField h_cell_(kappa(Twall) * patch().deltaCoeffs());

        forAll(Twall, i)
        {
            const scalar h1 = h_cell_[i];
            const scalar h2 = h_neighbour_[i];
            const scalar T2 = T_neighbour_[i];

            const scalar h2T2 = h2 * T2;

            if (qr[i] < 0.0)
            {
                // qr < 0 := cooling wall by radiation flux (into the fluid region)
                const scalar h2_qr = h2 - qr[i] / Twall[i];

                value[i] = h2T2 / h2_qr;
                fract[i] = h2_qr / (h2_qr + h1);
            }
            else
            {
                // qr >= 0 := heating wall with the incomming radiation flux
                value[i] = (h2T2 + qr[i]) / h2;
                fract[i] = h2 / (h2 + h1);
            }
        }

        //
        value = relaxation_ * value + (1 - relaxation_) * refValue0;
        fract = relaxation_ * fract + (1 - relaxation_) * valueFraction0;

        //
        refGrad() = 0;
        break;
    }

    //
    mixedFvPatchScalarField::updateCoeffs();

    //
    DebugInfo
        << patch().boundaryMesh().mesh().name() << ':' << patch().name() << ':' << nl
        << internalField().name() << " :" << nl
        << "\t- heat transfer rate:" << gSum(kappa(Twall) * patch().magSf() * snGrad()) << nl
        << "\t- radiation flux:" << gSum(qrPrevious_) << nl
        << "\t- wall heat flux:" << gSum(getWallHeatFlux()) << nl
        << "\t- wall temperature:" << nl
        << "\t\t- min:" << gMin(*this) << nl
        << "\t\t- max:" << gMax(*this) << nl
        << "\t\t- avg:" << gAverage(*this) << nl;
}

void Foam::apiCoupledTemperatureFvPatchScalarField::write
(
    Ostream& os
) const
{
    fvPatchScalarField::write(os);
    os.writeEntry("mode", operationModeNames[mode_]);
    temperatureCoupledBase::write(os);

    T_neighbour_.writeEntry("T_neighbour", os);
    h_neighbour_.writeEntry("h_neighbour", os);
    heatflux_.writeEntry("heatflux", os);

    if (relaxation_ < 1)
        os.writeEntry("relaxation", relaxation_);

    os.writeEntry("qr", qrName_);

    if (qrName_ != "none")
    {
        os.writeEntry("qrRelaxation", qrRelaxation_);
        qrPrevious_.writeEntry("qrPrevious", os);
    }

    refValue().writeEntry("refValue", os);
    refGrad().writeEntry("refGradient", os);
    valueFraction().writeEntry("valueFraction", os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        apiCoupledTemperatureFvPatchScalarField
    );
}

// ************************************************************************* //
