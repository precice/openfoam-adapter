#include "HeatFlux.H"

using namespace Foam;

preciceAdapter::User::HeatFlux::HeatFlux
(
    volScalarField * T,
    double k
)
:
T_(T),
k_(k)
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

        // TODO: Compute the heat flux on the patch
        // Q = - k * gradient(T)
        scalarField heatFlux
        =
        -k_ *
        refCast<fixedValueFvPatchScalarField>
        (
            T_->boundaryFieldRef()[patchID]
        ).snGrad();

        // For every cell of the patch
        forAll(heatFlux, i)
        {
            // Copy the heat flux into the buffer
            buffer[bufferIndex++]
            =
            heatFlux[i];
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

        // Get the boundary field on which the gradient will be set
        fixedGradientFvPatchScalarField & gradientPatch
        =
        refCast<fixedGradientFvPatchScalarField>
        (
            T_->boundaryFieldRef()[patchID]
        );

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Compute and assign the gradient from the buffer.
            // The sign of the heat flux needs to be inversed,
            // as the buffer contains the flux that enters the boundary:
            // gradient(T) = -Q / -k
            gradientPatch.gradient()[i]
            =
            buffer[bufferIndex++] / k_;
        }
    }
}
