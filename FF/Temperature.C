#include "Temperature.H"

using namespace Foam;

preciceAdapter::FF::Temperature::Temperature(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT)))
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::FF::Temperature::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);
        scalarField gradientPatch((T_->boundaryFieldRef()[patchID])
                                      .snGrad());

        // For every cell of the patch
        forAll(T_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++] =
                T_->boundaryFieldRef()[patchID][i];
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::Temperature::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);
        // For every cell of the patch
        forAll(T_->boundaryFieldRef()[patchID], i)
        {
            T_->boundaryFieldRef()[patchID][i] = buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::Temperature::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::Temperature::getDataName() const
{
    return "Temperature";
}
