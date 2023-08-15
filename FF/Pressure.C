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

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_->boundaryField()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++] =
                p_->boundaryField()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Pressure::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_->boundaryField()[patchID], i)
        {
            // Set the pressure as the buffer value
            p_->boundaryField()[patchID][i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::Pressure::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::Pressure::getDataName() const
{
    return "Pressure";
}
