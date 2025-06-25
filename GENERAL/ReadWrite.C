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

// only surface field coupling ATM (boundaries)

std::size_t preciceAdapter::MODULE::ScalarFieldCoupler::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    // TODO meshconnectivity

    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : scalarField_->internalField())
            {
                buffer[bufferIndex++] = cell;
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(scalarField_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // Copy the scalar value into the buffer
                    buffer[bufferIndex++] = scalarField_->internalField()[currentCell];
                }
            }
        }
    }

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

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (auto& cell : scalarField_->ref())
            {
                cell = buffer[bufferIndex++];
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(scalarField_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // Copy the scalar value from the buffer
                    scalarField_->ref()[currentCell] = buffer[bufferIndex++];
                }
            }
        }
    }

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
    // TODO

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
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::MODULE::ScalarFieldCoupler::getDataName() const
{
    return fieldConfig_.name;
}

//----- preciceAdapter::MODULE::VectorFieldCoupler -----------------------------------------

preciceAdapter::MODULE::VectorFieldCoupler::VectorFieldCoupler(
    const Foam::fvMesh& mesh,
    const struct fieldConfig& fieldConfig)
: vectorField_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(fieldConfig.solver_name))),
  mesh_(mesh),
  fieldConfig_(fieldConfig)
{
    dataType_ = vector;
}

std::size_t preciceAdapter::MODULE::VectorFieldCoupler::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (const auto& cell : vectorField_->internalField())
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
                cellSet overlapRegion(vectorField_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    buffer[bufferIndex++] = vectorField_->internalField()[currentCell].x();

                    // y-dimension
                    buffer[bufferIndex++] = vectorField_->internalField()[currentCell].y();

                    if (dim == 3)
                    {
                        // z-dimension
                        buffer[bufferIndex++] = vectorField_->internalField()[currentCell].z();
                    }
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the vector field boundary patch
        auto& boundaryPatch = vectorField_->boundaryField()[patchID];
        // For every cell of the patch
        forAll(boundaryPatch, i)
        {
            buffer[bufferIndex++] = boundaryPatch[i].x();

            buffer[bufferIndex++] = boundaryPatch[i].y();

            if (dim == 3)
            {
                buffer[bufferIndex++] = boundaryPatch[i].z();
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::MODULE::VectorFieldCoupler::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        if (cellSetNames_.empty())
        {
            for (auto& cell : vectorField_->ref())
            {
                // x-dimension
                cell.x() = buffer[bufferIndex++];

                // y-dimension
                cell.y() = buffer[bufferIndex++];

                if (dim == 3)
                {
                    // z-dimension
                    cell.z() = buffer[bufferIndex++];
                }
            }
        }
        else
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(vectorField_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    vectorField_->ref()[currentCell].x() = buffer[bufferIndex++];

                    // y-dimension
                    vectorField_->ref()[currentCell].y() = buffer[bufferIndex++];

                    if (dim == 3)
                    {
                        // z-dimension
                        vectorField_->ref()[currentCell].z() = buffer[bufferIndex++];
                    }
                }
            }
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the vector field boundary patch
        auto& boundaryPatch = vectorField_->boundaryFieldRef()[patchID];

        // For every cell of the patch
        forAll(boundaryPatch, i)
        {
            boundaryPatch[i].x() = buffer[bufferIndex++];

            boundaryPatch[i].y() = buffer[bufferIndex++];

            if (dim == 3)
            {
                boundaryPatch[i].z() = buffer[bufferIndex++];
            }
        }
    }
}

bool preciceAdapter::MODULE::VectorFieldCoupler::isLocationTypeSupported(const bool meshConnectivity) const
{
    // TODO
    if (meshConnectivity)
    {
        // does not support volume mapping with mesh connectivity
        return (this->locationType_ == LocationType::faceCenters);
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::MODULE::VectorFieldCoupler::getDataName() const
{
    return fieldConfig_.name;
}