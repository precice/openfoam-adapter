#include "PrghGradient.H"

using namespace Foam;

preciceAdapter::FF::PrghGradient::PrghGradient
(
    const Foam::fvMesh& mesh,
    const std::string namePrgh
)
:
p_rgh_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(namePrgh)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::FF::PrghGradient::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the pressure gradient boundary patch
        scalarField gradientPatch
        =
        refCast<fixedValueFvPatchScalarField>
        (
            p_rgh_->boundaryFieldRef()[patchID]
        ).snGrad();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Copy the pressure gradient into the buffer
            buffer[bufferIndex++]
            =
            -gradientPatch[i];
        }
    }
}

void preciceAdapter::FF::PrghGradient::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the pressure gradient boundary patch
        scalarField & gradientPatch
        =
        refCast<fixedGradientFvPatchScalarField>
        (
            p_rgh_->boundaryFieldRef()[patchID]
        ).gradient();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Set the pressure gradient as the buffer value
            gradientPatch[i]
            =
            buffer[bufferIndex++];
        }
    }
}
