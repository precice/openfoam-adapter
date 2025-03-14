#include "ReadWrite.H"
#include "primitivePatchInterpolation.H"
#include "fvCFD.H"

#include "fixedValueFvPatchFields.H"
#include "mixedFvPatchFields.H"

using namespace Foam;

//----- preciceAdapter::MODULE::scalarFieldCoupler -----------------------------------------

preciceAdapter::MODULE::ScalarFieldCoupler::ScalarFieldCoupler(
    const Foam::fvMesh& mesh,
    const struct fieldConfig& fieldConfig)
: scalarField_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(fieldConfig.solver_name))),
  mesh_(mesh),
  fieldConfig_(fieldConfig)
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::MODULE::ScalarFieldCoupler::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    // TODO meshconnectivity

    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        if (fieldConfig_.operation == "value")
        {
            const auto& boundaryPatch(scalarField_->boundaryField()[patchID]);

            // For every cell of the patch
            forAll(boundaryPatch, i)
            {
                buffer[bufferIndex++] = boundaryPatch[i];
            }
        }
        else if (fieldConfig_.operation == "gradient")
        {
            // for heat flux need to get value from boundary condition

            const scalarField gradientPatch((scalarField_->boundaryField()[patchID]).snGrad());

            // For every cell of the patch
            forAll(gradientPatch, i)
            {
                buffer[bufferIndex++] = -gradientPatch[i];
            }
        }
        else // TODO : raise error?
        {
            adapterInfo("Unsupported operation " + fieldConfig_.operation);
        }
    }
    return bufferIndex;
}

void preciceAdapter::MODULE::ScalarFieldCoupler::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        auto& bc = scalarField_->boundaryFieldRef()[patchID];

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
            // auto& boundaryPatch = bc;
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

        // // evaluate the boundary condition, i.e., do some calculation to obtain the actual value provided refValue
        // boundaryPatch.updateCoeffs();
        // boundaryPatch.evaluate();
    }
}

bool preciceAdapter::MODULE::ScalarFieldCoupler::isLocationTypeSupported(const bool meshConnectivity) const
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

std::string preciceAdapter::MODULE::ScalarFieldCoupler::getDataName() const
{
    return fieldConfig_.name;
}