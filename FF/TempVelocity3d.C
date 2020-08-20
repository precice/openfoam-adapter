#include "TempVelocity3d.H"

using namespace Foam;

preciceAdapter::FF::TempVelocity3d::TempVelocity3d
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

void preciceAdapter::FF::TempVelocity3d::write(double * buffer, bool meshConnectivity, const unsigned int dim)
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

            std::cout  << buffer[bufferIndex] << ", ";

            // y-dimension
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i].y();

            std::cout  << buffer[bufferIndex] << ", ";

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                =
                U_->boundaryFieldRef()[patchID][i].z();
                std::cout  << buffer[bufferIndex] << ", ";

        }
    }
}

void preciceAdapter::FF::TempVelocity3d::read(double * buffer, const unsigned int dim)
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
            U_->boundaryFieldRef()[patchID][i].x()
            =
            buffer[bufferIndex++];

            // y-dimension
            U_->boundaryFieldRef()[patchID][i].y()
            =
            buffer[bufferIndex++];

            if(dim == 3)
                // z-dimension
                U_->boundaryFieldRef()[patchID][i].z()
                =
                buffer[bufferIndex++];
        }
    }
}
