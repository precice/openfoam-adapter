
#include "apiCoupledTemperatureFvPatchScalarField.H"

#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "physicoChemicalConstants.H"


// * * * * * * * * * * * * * * *  Static Members   * * * * * * * * * * * * * //


#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotCOM) && (OpenFOAM_VERSION_MAJOR >= 1706)
const Foam::Enum
<
    Foam::apiCoupledTemperatureFvPatchScalarField::operationMode
>
Foam::apiCoupledTemperatureFvPatchScalarField::operationModeNames
({
    { operationMode::mixedBC, "mixed" },
    { operationMode::fixedTemperature, "fixedTemperature" },
    { operationMode::fixedHeatFlux, "fixedHeatFlux" },
    { operationMode::fixedMixedTemperatureHTC, "HTC" },
});
#else
namespace Foam
{
    template<>
    const char*
    NamedEnum
    <
        apiCoupledTemperatureFvPatchScalarField::operationMode,
        4
    >::names[] =
    {
        "mixed",
        "fixedTemperature",
        "fixedHeatFlux",
        "HTC"
    };
}

const Foam::NamedEnum
<
    Foam::apiCoupledTemperatureFvPatchScalarField::operationMode,
    4
> Foam::apiCoupledTemperatureFvPatchScalarField::operationModeNames;
#endif


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
	mixedFvPatchScalarField(p, iF),

#if ((OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR <= 7)) || \
    ((OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotCOM) && (OpenFOAM_VERSION_MAJOR <= 1906))
    temperatureCoupledBase
    (
        patch(),
        "undefined",
        "undefined",
        "undefined-K"
    ),
#elif OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG
    temperatureCoupledBase(patch()),
#else
    temperatureCoupledBase
    (
        patch(),
        "undefined",
        "undefined",
        "undefined-K",
        "undefined-alpha"
    ),
#endif
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
#if OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG && (OpenFOAM_VERSION_MAJOR >= 7)
    switch (mode_)
    {
    case fixedHeatFlux:
        mapper(heatflux_, rhs.heatflux_);
        break;

    case fixedMixedTemperatureHTC:
        mapper(T_neighbour_, rhs.T_neighbour_);
        mapper(h_neighbour_, rhs.h_neighbour_);
        break;

    default:
        break;
    }
    
    if (qrName_ != "none")
    {
        mapper(qrPrevious_, rhs.qrPrevious_);
    }
#elif OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG
    switch (mode_)
    {
    case fixedHeatFlux:
        heatflux_.setSize(mapper.size());
        heatflux_.map(rhs.heatflux_, mapper);
        break;

    case fixedMixedTemperatureHTC:
        T_neighbour_.setSize(mapper.size());
        T_neighbour_.map(rhs.T_neighbour_, mapper);

        h_neighbour_.setSize(mapper.size());
        h_neighbour_.map(rhs.h_neighbour_, mapper);
        break;

    default:
        break;
    }
    
    if (qrName_ != "none")
    {
        qrPrevious_.setSize(mapper.size());
        qrPrevious_.map(rhs.qrPrevious_, mapper);
    }
#else
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

    default:
        break;
    }
    
    if (qrName_ != "none")
    {
        qrPrevious_.resize(mapper.size());
        qrPrevious_.map(rhs.qrPrevious_, mapper);
    }
#endif
}


Foam::apiCoupledTemperatureFvPatchScalarField::
apiCoupledTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
	mixedFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), dict),

#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotCOM) && (OpenFOAM_VERSION_MAJOR >= 1706)
    mode_(operationModeNames.get("mode", dict)),
    qrName_(dict.getOrDefault<word>("qr", "none")),
    relaxation_(dict.getOrDefault<scalar>("relaxation", scalar(1))),
    qrRelaxation_(dict.getOrDefault<scalar>("qrRelaxation", scalar(1)))
#else
    mode_(operationModeNames.read(dict.lookup("mode"))),
    qrName_(dict.lookupOrDefault<word>("qr", "none")),
    relaxation_(dict.lookupOrDefault<scalar>("relaxation", scalar(1))),
    qrRelaxation_(dict.lookupOrDefault<scalar>("qrRelaxation", scalar(1)))
#endif
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
        if (dict.found("heatFlux"))
            heatflux_ = scalarField("heatFlux", dict, p.size());
        else
            heatflux_.resize(p.size(), scalar(0));

        if(!dict.found("refValue"))
                refValue() = 0;
        
        if(!dict.found("valueFraction"))
            valueFraction() = 0;
        break;

    case fixedMixedTemperatureHTC:
        if (dict.found("h_Neighbour"))
            h_neighbour_ = scalarField("h_Neighbour_", dict, p.size());
        else
            h_neighbour_.resize(p.size(), scalar(1e-6));
        
        if (dict.found("T_Neighbour"))
            T_neighbour_ = scalarField("T_Neighbour", dict, p.size());
        else
            T_neighbour_.resize(p.size(), scalar(1));

        if(!dict.found("refGradient"))
            refGrad() = 0;

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
        
    default:
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

#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR >= 7)
    switch (mode_)
    {
    case fixedHeatFlux:
        mapper(heatflux_, heatflux_);
        break;
    case fixedMixedTemperatureHTC:
        mapper(T_neighbour_, T_neighbour_);
        mapper(h_neighbour_, h_neighbour_);
        break;
    default:
        break;
    }

    if (qrName_ != "none")
        mapper(qrPrevious_, qrPrevious_);
#else
    switch (mode_)
    {
    case fixedHeatFlux:
        heatflux_.autoMap(mapper);
        break;
    case fixedMixedTemperatureHTC:
        T_neighbour_.autoMap(mapper);
        h_neighbour_.autoMap(mapper);
        break;
    default:
        break;
    }

    if (qrName_ != "none")
        qrPrevious_.autoMap(mapper);
#endif
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
    default:
        break;
    }

    if (qrName_ != "none")
        qrPrevious_.rmap(rhs.qrPrevious_, addr);
}

Foam::tmp<Foam::scalarField> Foam::apiCoupledTemperatureFvPatchScalarField::getWallHeatFlux
() const
{
    auto heatflux (kappa(*this) * snGrad());

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
    return kappa(*this) * patch().deltaCoeffs();
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

void Foam::apiCoupledTemperatureFvPatchScalarField::updateCoeffs
()
{
    // stop if up-to-date
    if (updated()) return;

    // update coeff's needed?
    if(!(mode_ == fixedHeatFlux || mode_ == fixedMixedTemperatureHTC))
    {
        mixedFvPatchScalarField::updateCoeffs();
        return;
    }

    // qr field
    if (qrName_ != "none")
    {
        const auto data = qrRelaxation_ * patch().lookupPatchField<volScalarField, scalar>(qrName_) + (1 - qrRelaxation_) * qrPrevious_;

        // copy to
        qrPrevious_ = data.ref();
    };

    //
    const scalarField&  qr      (qrPrevious_);

    // do update depending on operation mode
    switch (mode_)
    {
    case fixedHeatFlux:
        refGrad() = (heatflux_ + qr) / kappa(*this);
        break;

    case fixedMixedTemperatureHTC:
    {
        // get values from mixed-value boundary field
        scalarField &value(refValue());
        scalarField &fract(valueFraction());

        const scalarField refValue0(value);
        const scalarField valueFraction0(fract);
        const scalarField h_cell(kappa(*this) * patch().deltaCoeffs());

        const scalarField&  Twall   (*this);
        forAll(Twall, i)
        {
            const scalar h1 = h_cell[i];
            const scalar h2 = h_neighbour_[i];
            const scalar T2 = T_neighbour_[i];

            const scalar h2T2 = h2 * T2;

            if (qr[i] < 0.0)
            {
                // case 2
                const scalar h2_qr = h2 - qr[i] / Twall[i];

                value[i] = h2T2 / h2_qr;
                fract[i] = h2_qr / (h2_qr + h1);
            }
            else
            {
                // case 1
                value[i] = (h2T2 + qr[i]) / h2;
                fract[i] = h2 / (h2 + h1);
            }
        }

        //
        value = relaxation_ * value + (1 - relaxation_) * refValue0;
        fract = relaxation_ * fract + (1 - relaxation_) * valueFraction0;
        break;
    }   
    default: break;
    }

    //
    mixedFvPatchScalarField::updateCoeffs();

    //
    DebugInfo
        << patch().boundaryMesh().mesh().name() << ':' << patch().name() << ':' << nl
        << internalField().name() << " :" << nl
        << "\t- heat transfer rate:" << gSum(kappa(*this) * patch().magSf() * snGrad()) << nl
        << "\t- radiation flux:" << gSum(qrPrevious_) << nl
        << "\t- wall heat flux:" << gSum(getWallHeatFlux()) << nl
        << "\t- wall temperature:" << nl
        << "\t\t- min:" << gMin(*this) << nl
        << "\t\t- max:" << gMax(*this) << nl
        << "\t\t- avg:" << gAverage(*this) << endl;
}

void Foam::apiCoupledTemperatureFvPatchScalarField::write
(
    Ostream& os
) const
{
    fvPatchScalarField::write(os);

#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR >= 7)
    writeEntry(os, "mode", operationModeNames[mode_]);
#elif (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG)
    os.writeKeyword("mode") << operationModeNames[mode_] << token::END_STATEMENT << nl;
#else
    os.writeEntry("mode", operationModeNames[mode_]);
#endif

    temperatureCoupledBase::write(os);

#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR >= 7)
    writeEntry(os, "T_neighbour", T_neighbour_);
    writeEntry(os, "h_neighbour", h_neighbour_);
    writeEntry(os, "heatflux", heatflux_);
#else
    T_neighbour_.writeEntry("T_neighbour", os);
    h_neighbour_.writeEntry("h_neighbour", os);
    heatflux_.writeEntry("heatflux", os);
#endif

#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR >= 7)
    if (relaxation_ < 1)
        writeEntry(os, "relaxation", relaxation_);

    writeEntry(os, "qr", qrName_);

    if (qrName_ != "none")
    {
        writeEntry(os, "qrRelaxation", qrRelaxation_);
        writeEntry(os, "qrPrevious", qrPrevious_);
    }
#elif (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG)
    if (relaxation_ < 1)
        os.writeKeyword("relaxation") << relaxation_ << token::END_STATEMENT << nl;

    os.writeKeyword("qr") << qrName_ << token::END_STATEMENT << nl;

    if (qrName_ != "none")
    {
        os.writeKeyword("qrRelaxation") << qrRelaxation_ << token::END_STATEMENT << nl;
        qrPrevious_.writeEntry("qrPrevious", os);
    }
#else
    if (relaxation_ < 1)
        os.writeEntry("relaxation", relaxation_);

    os.writeEntry("qr", qrName_);

    if (qrName_ != "none")
    {
        os.writeEntry("qrRelaxation", qrRelaxation_);
        qrPrevious_.writeEntry("qrPrevious", os);
    }
#endif

#if ((OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR >= 7))
    writeEntry(os, "refValue", refValue());
    writeEntry(os, "refGradient", refGrad());
    writeEntry(os, "valueFraction", valueFraction());
    writeEntry(os, "value", *this);
#else
    refValue().writeEntry("refValue", os);
    refGrad().writeEntry("refGradient", os);
    valueFraction().writeEntry("valueFraction", os);
    writeEntry("value", os);
#endif
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
