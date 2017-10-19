#include "HeatFlux.H"

#include "fvCFD.H"

using namespace Foam;

//----- preciceAdapter::User::HeatFlux -----------------------------------------

preciceAdapter::User::HeatFlux::HeatFlux
(
    const Foam::fvMesh& mesh
)
:
T_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>("T")
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::User::HeatFlux::write(double * buffer)
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

void preciceAdapter::User::HeatFlux::read(double * buffer)
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

//----- preciceAdapter::User::HeatFlux_Compressible ----------------------------

preciceAdapter::User::HeatFlux_Compressible::HeatFlux_Compressible
(
    const Foam::fvMesh& mesh
)
:
HeatFlux(mesh),
Kappa_(new KappaEff_Compressible(mesh))
{
}

preciceAdapter::User::HeatFlux_Compressible::~HeatFlux_Compressible()
{
    delete Kappa_;
}

void preciceAdapter::User::HeatFlux_Compressible::extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::User::HeatFlux_Compressible::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

//----- preciceAdapter::User::HeatFlux_Incompressible --------------------------

preciceAdapter::User::HeatFlux_Incompressible::HeatFlux_Incompressible
(
    const Foam::fvMesh& mesh
)
:
HeatFlux(mesh),
Kappa_(new KappaEff_Incompressible(mesh))
{
}

preciceAdapter::User::HeatFlux_Incompressible::~HeatFlux_Incompressible()
{
    delete Kappa_;
}

void preciceAdapter::User::HeatFlux_Incompressible::extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::User::HeatFlux_Incompressible::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

//----- preciceAdapter::User::HeatFlux_Basic -----------------------------------

preciceAdapter::User::HeatFlux_Basic::HeatFlux_Basic
(
    const Foam::fvMesh& mesh
)
:
HeatFlux(mesh),
Kappa_(new KappaEff_Basic(mesh))
{
}

preciceAdapter::User::HeatFlux_Basic::~HeatFlux_Basic()
{
    delete Kappa_;
}

void preciceAdapter::User::HeatFlux_Basic::extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::User::HeatFlux_Basic::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}
