#include "PressureGradientFull.H"

using namespace Foam;

preciceAdapter::FF::PressureGradientFull::PressureGradientFull(
    const Foam::fvMesh& mesh,
    const std::string nameP)
: p_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameP))),
  gradP_(IOobject(
             "gradP",
             mesh.time().timeName(),
             mesh,
             IOobject::NO_READ,
             IOobject::NO_WRITE),
         fvc::grad(*p_))
{
    dataType_ = vector;
}

std::size_t preciceAdapter::FF::PressureGradientFull::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;
    gradP_ = fvc::grad(*p_);

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : gradP_.internalField())
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
                cellSet overlapRegion(p_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    buffer[bufferIndex++] = gradP_.internalField()[currentCell].x();

                    // y-dimension
                    buffer[bufferIndex++] = gradP_.internalField()[currentCell].y();

                    if (dim == 3)
                    {
                        // z-dimension
                        buffer[bufferIndex++] = gradP_.internalField()[currentCell].z();
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
        forAll(gradP_.boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++] =
                gradP_.boundaryFieldRef()[patchID][i].x();

            // y-dimension
            buffer[bufferIndex++] =
                gradP_.boundaryFieldRef()[patchID][i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    gradP_.boundaryFieldRef()[patchID][i].z();
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::PressureGradientFull::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the pressure gradient boundary patch
        scalarField& gradientPatch =
            refCast<fixedGradientFvPatchScalarField>(
                p_->boundaryFieldRef()[patchID])
                .gradient();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Set the pressure gradient as the buffer value
            gradientPatch[i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::PressureGradientFull::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
}

std::string preciceAdapter::FF::PressureGradientFull::getDataName() const
{
    return "PressureGradientFull";
}
