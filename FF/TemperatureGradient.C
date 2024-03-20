#include "TemperatureGradient.H"
#include "mixedFvPatchFields.H"

using namespace Foam;

preciceAdapter::FF::TemperatureGradient::TemperatureGradient(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT)))
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::FF::TemperatureGradient::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the Temperature gradient boundary patch
        const scalarField gradientPatch((T_->boundaryFieldRef()[patchID])
                                            .snGrad());

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Copy the Temperature gradient into the buffer
            buffer[bufferIndex++] =
                gradientPatch[i];
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::TemperatureGradient::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the Temperature gradient boundary patch
        scalarField& gradientPatch =
            refCast<fixedGradientFvPatchScalarField>(
                T_->boundaryFieldRef()[patchID])
                .gradient();

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Set the Temperature gradient as the buffer value
            gradientPatch[i] =
                -buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::TemperatureGradient::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::TemperatureGradient::getDataName() const
{
    return "TemperatureGradient";
}
