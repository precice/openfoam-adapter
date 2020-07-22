#include "AlphaGradient.H"

using namespace Foam;

preciceAdapter::FF::AlphaGradient::AlphaGradient
(
    const Foam::fvMesh& mesh,
    const std::string nameA
)
:
alpha_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameA)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::FF::AlphaGradient::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the Alpha gradient boundary patch
        scalarField gradientPatch
        =
        refCast<fixedValueFvPatchScalarField>
        (
            alpha_->boundaryFieldRef()[patchID]
        ).snGrad();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Copy the Alpha gradient into the buffer
            buffer[bufferIndex++]
            =
            -gradientPatch[i];
        }
    }
}

void preciceAdapter::FF::AlphaGradient::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the Alpha gradient boundary patch
        scalarField & gradientPatch
        =
        refCast<fixedGradientFvPatchScalarField>
        (
            alpha_->boundaryFieldRef()[patchID]
        ).gradient();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Set the Alpha gradient as the buffer value
            gradientPatch[i]
            =
            buffer[bufferIndex++];
        }
    }
}
