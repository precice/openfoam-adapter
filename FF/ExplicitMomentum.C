#include "ExplicitMomentum.H"
#include "coupledVelocityFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::ExplicitMomentum::ExplicitMomentum(
    const Foam::fvMesh& mesh,
    const std::string nameExplicitMomentum)
{
    if (mesh.foundObject<volVectorField>(nameExplicitMomentum))
    {
        adapterInfo("Loaded existing explicit momentum object " + nameExplicitMomentum, "debug");
        ExplicitMomentum_ = const_cast<volVectorField*>(
            &mesh.lookupObject<volVectorField>(nameExplicitMomentum));
    }
    else
    {
        adapterInfo("Creating a new explicit momentum object " + nameExplicitMomentum, "debug");
        ExplicitMomentum_ = new volVectorField(
            IOobject(
                nameExplicitMomentum,
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE),
            mesh);
    }
    dataType_ = vector;
}

std::size_t preciceAdapter::FF::ExplicitMomentum::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : ExplicitMomentum_->internalField())
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
                cellSet overlapRegion(ExplicitMomentum_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    buffer[bufferIndex++] = ExplicitMomentum_->internalField()[currentCell].x();

                    // y-dimension
                    buffer[bufferIndex++] = ExplicitMomentum_->internalField()[currentCell].y();

                    if (dim == 3)
                    {
                        // z-dimension
                        buffer[bufferIndex++] = ExplicitMomentum_->internalField()[currentCell].z();
                    }
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        vectorField ExplicitMomentumPatch = ExplicitMomentum_->boundaryField()[patchID];

        // For every cell of the patch
        forAll(ExplicitMomentum_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++] =
                ExplicitMomentumPatch[i].x();

            // y-dimension
            buffer[bufferIndex++] =
                ExplicitMomentumPatch[i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    ExplicitMomentumPatch[i].z();
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::ExplicitMomentum::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (auto& cell : ExplicitMomentum_->ref())
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
                cellSet overlapRegion(ExplicitMomentum_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    ExplicitMomentum_->ref()[currentCell].x() = buffer[bufferIndex++];

                    // y-dimension
                    ExplicitMomentum_->ref()[currentCell].y() = buffer[bufferIndex++];

                    if (dim == 3)
                    {
                        // z-dimension
                        ExplicitMomentum_->ref()[currentCell].z() = buffer[bufferIndex++];
                    }
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity value boundary patch
        vectorField* valuePatchPtr = &ExplicitMomentum_->boundaryFieldRef()[patchID];
        vectorField& valuePatch = *valuePatchPtr;

        // For every cell of the patch
        forAll(ExplicitMomentum_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            valuePatch[i].x() =
                buffer[bufferIndex++];

            // y-dimension
            valuePatch[i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                valuePatch[i].z() =
                    buffer[bufferIndex++];
            }
        }
    }
}

bool preciceAdapter::FF::ExplicitMomentum::isLocationTypeSupported(const bool meshConnectivity) const
{
    if (meshConnectivity)
    {
        return false;
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::FF::ExplicitMomentum::getDataName() const
{
    return "ExplicitMomentum";
}
