#include "Prgh.H"

using namespace Foam;

preciceAdapter::FF::Prgh::Prgh
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

void preciceAdapter::FF::Prgh::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_rgh_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++]
            =
            p_rgh_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Prgh::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_rgh_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            p_rgh_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }
    }
}
