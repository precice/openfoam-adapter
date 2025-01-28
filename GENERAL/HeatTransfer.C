#include "HeatTransfer.H"
#include "primitivePatchInterpolation.H"
#include "fvCFD.H"

#include "apiCoupledTemperatureFvPatchScalarField.H"


using namespace Foam;

//----- preciceAdapter::MODULE::Temperature -----------------------------------------

preciceAdapter::MODULE::Temperature::Temperature(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::MODULE::Temperature::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const auto& boundaryPatch(T_->boundaryField()[patchID]);

        // // If we use the mesh connectivity, we interpolate from the centres to the nodes
        // if (meshConnectivity)
        // {
        //     //Setup Interpolation object
        //     primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        //     //Interpolate
        //     auto boundaryPatchPoints = patchInterpolator.faceToPointInterpolate(boundaryPatch);

        //     // For every cell of the patch
        //     forAll(boundaryPatchPoints, i)
        //     {
        //         buffer[bufferIndex++] = boundaryPatchPoints[i];
        //     }
        // }
        // else
        // {

        // For every cell of the patch
        forAll(boundaryPatch, i)
        {
            buffer[bufferIndex++] = boundaryPatch[i];
        }

        // }
    }
    return bufferIndex;
}

void preciceAdapter::MODULE::Temperature::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // TODO: if (this->locationType_ == LocationType::volumeCenters) ?

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        auto& boundaryPatch(refCast<apiCoupledTemperatureFvPatchScalarField>(T_->boundaryFieldRef()[patchID]));
        auto& value = boundaryPatch.refValue();

        forAll(value, i)
        {
            value[i] = buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::MODULE::Temperature::isLocationTypeSupported(const bool meshConnectivity) const
{
    // For cases with mesh connectivity, we support:
    // - face nodes, only for writing
    // - face centers, only for reading
    // However, since we do not distinguish between reading and writing in the code, we
    // always return true and offload the handling to the user.
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::faceNodes); // we currently do not support meshConnectivity for volumeCenters
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters);
    }
}

std::string preciceAdapter::MODULE::Temperature::getDataName() const
{
    return "Temperature";
}


//----- preciceAdapter::MODULE::HeatFlux -----------------------------------------

preciceAdapter::MODULE::HeatFlux::HeatFlux(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::MODULE::HeatFlux::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const auto& gradientPatch(refCast<const apiCoupledTemperatureFvPatchScalarField>(T_->boundaryFieldRef()[patchID]));
        auto gradient(gradientPatch.getWallHeatFlux());

        const scalarField& data(gradient.cref());


        // // If we use the mesh connectivity, we interpolate from the centres to the nodes
        // if (meshConnectivity)
        // {
        //     //Setup Interpolation object
        //     primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        //     //Interpolate
        //     auto gradientPoints = patchInterpolator.faceToPointInterpolate(gradient);

        //     // For every cell of the patch
        //     forAll(gradientPoints, i)
        //     {
        //         buffer[bufferIndex++] = - gradientPoints[i];
        //     }
        // }
        // else
        // {

        // For every cell of the patch
        forAll(data, i)
        {
            buffer[bufferIndex++] = -data[i];
        }

        // }
    }
    return bufferIndex;
}

void preciceAdapter::MODULE::HeatFlux::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the temperature gradient boundary patch
        auto& gradientPatch(refCast<apiCoupledTemperatureFvPatchScalarField>(T_->boundaryFieldRef()[patchID]));
        auto& gradient = gradientPatch.heatFlux();

        // For every cell of the patch
        forAll(gradient, i)
        {
            gradient[i] = buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::MODULE::HeatFlux::isLocationTypeSupported(const bool meshConnectivity) const
{
    // For cases with mesh connectivity, we support:
    // - face nodes, only for writing
    // - face centers, only for reading
    // However, since we do not distinguish between reading and writing in the code, we
    // always return true and offload the handling to the user.
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::faceNodes); // we currently do not support meshConnectivity for volumeCenters
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters);
    }
}

std::string preciceAdapter::MODULE::HeatFlux::getDataName() const
{
    return "HeatFlux";
}
