#include "Phi.H"

using namespace Foam;

preciceAdapter::FF::Phi::Phi(
    const Foam::fvMesh& mesh,
    const std::string namePhi)
: phi_(
    const_cast<surfaceScalarField*>(
        &mesh.lookupObject<surfaceScalarField>(namePhi)))
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::FF::Phi::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(phi_->boundaryFieldRef()[patchID], i)
        {
            // Copy the Phi into the buffer
            buffer[bufferIndex++] =
                phi_->boundaryFieldRef()[patchID][i];
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::Phi::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        forAll(phi_->boundaryFieldRef()[patchID], i)
        {
            phi_->boundaryFieldRef()[patchID][i] = -buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::Phi::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::Phi::getDataName() const
{
    return "Phi";
}
