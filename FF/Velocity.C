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

    if (this->locationType_ == LocationType::volumeCenters)
    {
        forAll(U_->ref(), i)
        {
            // x-dimension
            buffer[bufferIndex++] = U_->ref()[i].x();

            // y-dimension
            buffer[bufferIndex++] = U_->ref()[i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] = U_->ref()[i].z();
            }
        }
    }

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
}

void preciceAdapter::FF::Velocity::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        forAll(U_->ref(), i)
        {
            // x-dimension
            U_->ref()[i].x() = buffer[bufferIndex++];

            // y-dimension
            U_->ref()[i].y() = buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                U_->ref()[i].z() = buffer[bufferIndex++];
            }
        }
    }

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
}

bool preciceAdapter::FF::Velocity::isLocationTypeSupported(const bool meshConnectivity) const
{
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceCenters);
    }
    else 
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::FF::Velocity::getDataName() const
{
    return "Velocity";
}
