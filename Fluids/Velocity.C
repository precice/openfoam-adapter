#include "Velocity.H"

using namespace Foam;

preciceAdapter::Fluids::Velocity::Velocity
(
    const Foam::fvMesh& mesh,
    const std::string nameT
)
:
U_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameT)
    )
)
{
    dataType_ = vector;
}

void preciceAdapter::Fluids::Velocity::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the three components of the velocity into the buffer
            buffer[bufferIndex++] = U_->boundaryFieldRef()[patchID][i].x();
            buffer[bufferIndex++] = U_->boundaryFieldRef()[patchID][i].y();
            buffer[bufferIndex++] = U_->boundaryFieldRef()[patchID][i].z();
        }
    }
}

void preciceAdapter::Fluids::Velocity::read(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            U_->boundaryFieldRef()[patchID][i].x() = buffer[bufferIndex++];
            U_->boundaryFieldRef()[patchID][i].y() = buffer[bufferIndex++];
            U_->boundaryFieldRef()[patchID][i].z() = buffer[bufferIndex++];
        }
    }
}
