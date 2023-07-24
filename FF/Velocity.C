#include "Velocity.H"

using namespace Foam;

preciceAdapter::FF::Velocity::Velocity(
    const Foam::fvMesh& mesh,
    const std::string nameU)
{
    if (nameU.compare("U_vol") == 0) {
        U_ = new volVectorField(
            IOobject
            (
                "U_vol",
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            ),
            mesh
        );
    }
    else {
        U_ = const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(nameU));
    }
    dataType_ = vector;
}

void preciceAdapter::FF::Velocity::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : U_->internalField())
            {
                // x-dimension
                buffer[bufferIndex++] = cell.x();

                // y-dimension
                buffer[bufferIndex++] = cell.y();

                if (dim == 3)
                {
                    // z-dimension
                    buffer[bufferIndex++] = cell.z();
                }
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(U_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    buffer[bufferIndex++] = U_->internalField()[currentCell].x();

                    // y-dimension
                    buffer[bufferIndex++] = U_->internalField()[currentCell].y();

                    if (dim == 3)
                    {
                        // z-dimension
                        buffer[bufferIndex++] = U_->internalField()[currentCell].z();
                    }
                }
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
        if (cellSetNames_.empty())
        {
            for (auto& cell : U_->ref())
            {
                // x-dimension
                cell.x() = buffer[bufferIndex++];

                // y-dimension
                cell.y() = buffer[bufferIndex++];

                if (dim == 3)
                {
                    // z-dimension
                    cell.z() = buffer[bufferIndex++];
                }
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(U_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    U_->ref()[currentCell].x() = buffer[bufferIndex++];

                    // y-dimension
                    U_->ref()[currentCell].y() = buffer[bufferIndex++];

                    if (dim == 3)
                    {
                        // z-dimension
                        U_->ref()[currentCell].z() = buffer[bufferIndex++];
                    }
                }
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
