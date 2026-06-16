#include "ImplicitMomentum.H"
#include "coupledVelocityFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::ImplicitMomentum::ImplicitMomentum(
    const Foam::fvMesh& mesh,
    const std::string nameImplicitMomentum)
{
    if (mesh.foundObject<volScalarField>(nameImplicitMomentum))
    {
        adapterInfo("Loaded existing implicit momentum object " + nameImplicitMomentum, "debug");
        ImplicitMomentum_ = const_cast<volScalarField*>(
            &mesh.lookupObject<volScalarField>(nameImplicitMomentum));
    }
    else
    {
        adapterInfo("Creating a new implicit momentum object " + nameImplicitMomentum, "debug");
        ImplicitMomentum_ = new volScalarField(
            IOobject(
                nameImplicitMomentum,
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE),
            mesh);
    }
    dataType_ = scalar;
}

std::size_t preciceAdapter::FF::ImplicitMomentum::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : ImplicitMomentum_->internalField())
            {
                buffer[bufferIndex++] = cell;
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(ImplicitMomentum_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    buffer[bufferIndex++] = ImplicitMomentum_->internalField()[currentCell];
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        scalarField ImplicitMomentumPatch = ImplicitMomentum_->boundaryField()[patchID];

        // For every cell of the patch
        forAll(ImplicitMomentum_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            buffer[bufferIndex++] =
                ImplicitMomentumPatch[i];
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::ImplicitMomentum::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (auto& cell : ImplicitMomentum_->ref())
            {
                cell = buffer[bufferIndex++];
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(ImplicitMomentum_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    ImplicitMomentum_->ref()[currentCell] = buffer[bufferIndex++];
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity value boundary patch
        scalarField* valuePatchPtr = &ImplicitMomentum_->boundaryFieldRef()[patchID];
        scalarField& valuePatch = *valuePatchPtr;

        // For every cell of the patch
        forAll(ImplicitMomentum_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            valuePatch[i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::ImplicitMomentum::isLocationTypeSupported(const bool meshConnectivity) const
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

std::string preciceAdapter::FF::ImplicitMomentum::getDataName() const
{
    return "ImplicitMomentum";
}
