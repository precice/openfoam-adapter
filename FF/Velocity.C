#include "Velocity.H"

using namespace Foam;

preciceAdapter::FF::Velocity::Velocity(
    const Foam::fvMesh& mesh,
    const std::string nameU)
: U_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(nameU)))
{
    dataType_ = vector;
}

void preciceAdapter::FF::Velocity::write(double* buffer, bool meshConnectivity, const unsigned int dim)
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
            buffer[bufferIndex++] =
                U_->boundaryFieldRef()[patchID][i].x();

            // y-dimension
            buffer[bufferIndex++] =
                U_->boundaryFieldRef()[patchID][i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    U_->boundaryFieldRef()[patchID][i].z();
            }
        }
    }

    if (this->locationType_ == LocationType::volume)
    {
        forAll(U_->ref(), k)
        {
            // x-dimension
            buffer[bufferIndex++] = U_->ref()[k].x();

            // y-dimension
            buffer[bufferIndex++] = U_->ref()[k].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] = U_->ref()[k].z();
            }   
        }
    }
}

void preciceAdapter::FF::Velocity::read(double* buffer, const unsigned int dim)
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
            U_->boundaryFieldRef()[patchID][i].x() =
                buffer[bufferIndex++];

            // y-dimension
            U_->boundaryFieldRef()[patchID][i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                U_->boundaryFieldRef()[patchID][i].z() =
                    buffer[bufferIndex++];
            }
        }
    }

    if (this->locationType_ == LocationType::volume)
    {
        forAll(U_->ref(), k)
        {
            // x-dimension
            U_->ref()[k].x() = buffer[bufferIndex++];

            // y-dimension
            U_->ref()[k].y() = buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                U_->ref()[k].z() = buffer[bufferIndex++];
            }
        }
    }
}

bool preciceAdapter::FF::Velocity::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::Velocity::getDataName() const
{
    return "Velocity";
}
