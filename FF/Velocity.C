#include "Velocity.H"

using namespace Foam;

preciceAdapter::FF::Velocity::Velocity
(
    const Foam::fvMesh& mesh,
    const std::string nameU
)
:
U_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameU)
    )
)
{
    dataType_ = scalar;
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
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i];
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
            U_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }
    }
}