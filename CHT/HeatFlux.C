#include "HeatFlux.H"

#include "fvCFD.H"

using namespace Foam;

//----- preciceAdapter::CHT::HeatFlux -----------------------------------------

preciceAdapter::CHT::HeatFlux::HeatFlux
(
    const Foam::fvMesh& mesh,
    const std::string nameT
)
:
T_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameT)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::CHT::HeatFlux::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        extractKappaEff(patchID);

        // Get the temperature gradient boundary patch
        scalarField gradientPatch
        =
        refCast<fixedValueFvPatchScalarField>
        (
            T_->boundaryFieldRef()[patchID]
        ).snGrad();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Copy the heat flux into the buffer
            // Q = - k * gradient(T)
            buffer[bufferIndex++]
            =
            -getKappaEffAt(i) * gradientPatch[i];
        }
    }
}

void preciceAdapter::CHT::HeatFlux::read(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        extractKappaEff(patchID);

        // Get the temperature gradient boundary patch
        scalarField & gradientPatch
        =
        refCast<fixedGradientFvPatchScalarField>
        (
            T_->boundaryFieldRef()[patchID]
        ).gradient();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Compute and assign the gradient from the buffer.
            // The sign of the heat flux needs to be inversed,
            // as the buffer contains the flux that enters the boundary:
            // gradient(T) = -Q / -k
            gradientPatch[i]
            =
            buffer[bufferIndex++] / getKappaEffAt(i);
        }
    }
}

//----- preciceAdapter::CHT::HeatFlux_Compressible ----------------------------

preciceAdapter::CHT::HeatFlux_Compressible::HeatFlux_Compressible
(
    const Foam::fvMesh& mesh,
    const std::string nameT
)
:
HeatFlux(mesh, nameT),
Kappa_(new KappaEff_Compressible(mesh))
{
}

preciceAdapter::CHT::HeatFlux_Compressible::~HeatFlux_Compressible()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatFlux_Compressible::extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::CHT::HeatFlux_Compressible::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

//----- preciceAdapter::CHT::HeatFlux_Incompressible --------------------------

preciceAdapter::CHT::HeatFlux_Incompressible::HeatFlux_Incompressible
(
    const Foam::fvMesh& mesh,
    const std::string nameT,
    const std::string nameTransportProperties,
    const std::string nameRho,
    const std::string nameCp,
    const std::string namePr,
    const std::string nameAlphat
)
:
HeatFlux(mesh, nameT),
Kappa_(new KappaEff_Incompressible(mesh, nameTransportProperties, nameRho, nameCp, namePr, nameAlphat))
{
}

preciceAdapter::CHT::HeatFlux_Incompressible::~HeatFlux_Incompressible()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatFlux_Incompressible::extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::CHT::HeatFlux_Incompressible::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

//----- preciceAdapter::CHT::HeatFlux_Basic -----------------------------------

preciceAdapter::CHT::HeatFlux_Basic::HeatFlux_Basic
(
    const Foam::fvMesh& mesh,
    const std::string nameT,
    const std::string nameTransportProperties,
    const std::string nameKappa
)
:
HeatFlux(mesh, nameT),
Kappa_(new KappaEff_Basic(mesh, nameTransportProperties, nameKappa))
{
}

preciceAdapter::CHT::HeatFlux_Basic::~HeatFlux_Basic()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatFlux_Basic::extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::CHT::HeatFlux_Basic::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}
