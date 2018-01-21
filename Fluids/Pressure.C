#include "Pressure.H"

using namespace Foam;

preciceAdapter::Fluids::Pressure::Pressure
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

void preciceAdapter::Fluids::Pressure::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(P_->boundaryFieldRef()[patchID], i)
        {
            // Copy the temperature into the buffer
            buffer[bufferIndex++]
            =
            P_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::Fluids::Pressure::read(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(P_->boundaryFieldRef()[patchID], i)
        {
            // Set the temperature as the buffer value
            P_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }
    }
}
