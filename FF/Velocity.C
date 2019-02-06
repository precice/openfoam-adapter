#include "Velocity.H"

using namespace Foam;

preciceAdapter::FF::Velocity::Velocity
(
    const Foam::fvMesh& mesh,
    const std::string nameU
)
:
U_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameU)
    )
)
{
    dataType_ = vector;
}

void preciceAdapter::FF::Velocity::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i].x();

            // y-dimension
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i].y();

            // z-dimension
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i].z();
        }
    }
}

void preciceAdapter::FF::Velocity::read(double * buffer)
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
            // x-dimension
            U_->boundaryFieldRef()[patchID][i][0]
            =
            buffer[bufferIndex++];

            // y-dimension
            U_->boundaryFieldRef()[patchID][i][1]
            =
            buffer[bufferIndex++];

            // z-dimension
            U_->boundaryFieldRef()[patchID][i][2]
            =
            buffer[bufferIndex++];
        }
    }
}