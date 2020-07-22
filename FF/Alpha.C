#include "Alpha.H"

using namespace Foam;

preciceAdapter::FF::Alpha::Alpha
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

void preciceAdapter::FF::Alpha::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++]
            =
            alpha_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Alpha::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            alpha_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }
    }
}
