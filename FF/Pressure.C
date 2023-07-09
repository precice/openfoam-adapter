#include "Pressure.H"

using namespace Foam;

preciceAdapter::FF::Pressure::Pressure(
    const Foam::fvMesh& mesh,
    const std::string nameP)
: p_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameP)))
{
    dataType_ = scalar;
}

void preciceAdapter::FF::Pressure::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        forAll(p_->internalField(), i)
        {
            buffer[bufferIndex++] = p_->internalField()[i];
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++] =
                p_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Pressure::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        forAll(p_->ref(), i)
        {
            p_->ref()[i] = buffer[bufferIndex++];
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            p_->boundaryFieldRef()[patchID][i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::Pressure::isLocationTypeSupported(const bool meshConnectivity) const
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

std::string preciceAdapter::FF::Pressure::getDataName() const
{
    return "Pressure";
}
