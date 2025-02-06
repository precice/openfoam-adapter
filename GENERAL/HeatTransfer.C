#include "HeatTransfer.H"
#include "primitivePatchInterpolation.H"
#include "fvCFD.H"

#include "fixedValueFvPatchFields.H"
#include "mixedFvPatchFields.H"

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

        // TODO meshconnectivity

        // For every cell of the patch
        forAll(boundaryPatch, i)
        {
            buffer[bufferIndex++] = boundaryPatch[i];
        }
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

        auto& bc = T_->boundaryFieldRef()[patchID];

        // Why do we need to cast at all? - Because boundaryField() returns fvPatchField instead of the derived boundary condition type
        // auto& boundaryPatch(refCast<apiCoupledTemperatureFvPatchScalarField>(T_->boundaryFieldRef()[patchID]));
        // auto& value = boundaryPatch.refValue();

        // Need to look into dynamic_cast

        if (isA<fixedValueFvPatchScalarField>(bc))
        {
            auto& boundaryPatch = refCast<fixedValueFvPatchScalarField>(bc);
            forAll(boundaryPatch, i)
            {
                boundaryPatch[i] = buffer[bufferIndex++];
            }
        }
        else if (isA<fixedGradientFvPatchScalarField>(bc))
        {
            // do nothing, this is handled in heat flux
            auto& boundaryPatch = bc;
        }
        else if (isA<mixedFvPatchScalarField>(bc))
        {
            auto& boundaryPatch = refCast<mixedFvPatchScalarField>(bc).refValue();
            forAll(boundaryPatch, i)
            {
                boundaryPatch[i] = buffer[bufferIndex++];
            }
        }
        else
        {
            FatalErrorInFunction << "Unsupported boundary condition type " << bc.type() << exit(FatalError);
        }

        // evaluate the boundary condition, i.e., do some calculation to obtain the actual value provided refValue
        // boundaryPatch.updateCoeffs();
        // boundaryPatch.evaluate();
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

        const auto& bc = T_->boundaryField()[patchID];

        if (isA<fixedValueFvPatchScalarField>(bc))
        {
            // do nothing, this is handled in temperature
        }
        else if (isA<fixedGradientFvPatchScalarField>(bc))
        {
            const auto& gradientPatch = refCast<const fixedGradientFvPatchScalarField>(bc);
            forAll(gradientPatch, i)
            {
                buffer[bufferIndex++] = -gradientPatch[i];
            }
        }
        else if (isA<mixedFvPatchScalarField>(bc))
        {
            const auto& gradientPatch = refCast<const mixedFvPatchScalarField>(bc).refGrad();
            forAll(gradientPatch, i)
            {
                buffer[bufferIndex++] = -gradientPatch[i];
            }
        }
        else
        {
            FatalErrorInFunction << "Unsupported boundary condition type " << bc.type() << exit(FatalError);
        }
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


        auto& bc = T_->boundaryFieldRef()[patchID];

        if (isA<fixedValueFvPatchScalarField>(bc))
        {
            // do nothing, this is handled in temperature
            auto& boundaryPatch = bc;
        }
        else if (isA<fixedGradientFvPatchScalarField>(bc))
        {
            auto& gradientPatch = refCast<fixedGradientFvPatchScalarField>(bc);
            forAll(gradientPatch, i)
            {
                gradientPatch[i] = buffer[bufferIndex++];
            }
        }
        else if (isA<mixedFvPatchScalarField>(bc))
        {
            auto& gradientPatch = refCast<mixedFvPatchScalarField>(bc).refGrad();
            forAll(gradientPatch, i)
            {
                gradientPatch[i] = buffer[bufferIndex++];
            }
        }
        else
        {
            FatalErrorInFunction << "Unsupported boundary condition type " << bc.type() << exit(FatalError);
        }

        // boundaryPatch.updateCoeffs();
        // boundaryPatch.evaluate();
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
