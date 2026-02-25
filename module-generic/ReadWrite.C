#include "ReadWrite.H"
#include "primitivePatchInterpolation.H"
#include "fvCFD.H"

#include "fixedValueFvPatchFields.H"
#include "fixedGradientFvPatchFields.H"

using namespace Foam;

//----- preciceAdapter::Generic::scalarFieldCoupler -----------------------------------------

preciceAdapter::Generic::ScalarFieldCoupler::ScalarFieldCoupler(
    const Foam::fvMesh& mesh,
    const preciceAdapter::FieldConfig& fieldConfig)
: scalarField_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(fieldConfig.solver_name))),
  mesh_(mesh),
  fieldConfig_(fieldConfig)
{
    dataType_ = scalar;
}

void preciceAdapter::Generic::ScalarFieldCoupler::initialize()
{
    if (fieldConfig_.operation == "surface-normal-gradient")
    {
        if (this->locationType_ != LocationType::faceCenters)
        {
            adapterInfo("Generic module: The surface-normal-gradient operation is only supported for faceCenters location type.", "error");
        }
    }

    if (fieldConfig_.operation == "gradient")
    {
        adapterInfo("Generic module: The gradient operation is not yet supported for scalar fields. Maybe you meant surface-normal-gradient?", "error");
    }
}


std::size_t preciceAdapter::Generic::ScalarFieldCoupler::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (fieldConfig_.operation == "surface-normal-gradient")
    {
        // For every boundary patch of the interface
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            int patchID = patchIDs_.at(j);

            // Get the surface normal gradient boundary patch
            scalarField gradientPatch(scalarField_->boundaryField()[patchID].snGrad());

            // For every cell of the patch
            forAll(gradientPatch, i)
            {
                buffer[bufferIndex++] = gradientPatch[i];
            }
        }
        return bufferIndex;
    }

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

        const auto& boundaryPatch(scalarField_->boundaryField()[patchID]);

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if (meshConnectivity)
        {
            //Create an Interpolation object at the boundary Field
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            //Interpolate from centers to nodes
            scalarField pointValues(
                patchInterpolator.faceToPointInterpolate(boundaryPatch));

            forAll(pointValues, i)
            {
                // Copy the scalar value into the buffer
                buffer[bufferIndex++] = pointValues[i];
            }
        }
        else
        {
            // For every cell of the patch
            forAll(boundaryPatch, i)
            {
                buffer[bufferIndex++] = boundaryPatch[i];
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::Generic::ScalarFieldCoupler::read(double* buffer, const unsigned int dim)
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
            auto& boundaryPatch = refCast<fixedGradientFvPatchScalarField>(bc);
            forAll(boundaryPatch, i)
            {
                boundaryPatch.gradient()[i] = buffer[bufferIndex++];
            }
        }
        else
        {
            adapterInfo("Generic module: Unsupported boundary condition type " + bc.type(), "error");
        }

        // // evaluate the boundary condition, i.e., do some calculation to obtain the actual value provided refValue
        // boundaryPatch.updateCoeffs();
        // boundaryPatch.evaluate();
    }
}

bool preciceAdapter::Generic::ScalarFieldCoupler::isLocationTypeSupported(const bool meshConnectivity) const
{
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceNodes);
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::Generic::ScalarFieldCoupler::getDataName() const
{
    return fieldConfig_.name;
}

//----- preciceAdapter::Generic::VectorFieldCoupler -----------------------------------------

preciceAdapter::Generic::VectorFieldCoupler::VectorFieldCoupler(
    const Foam::fvMesh& mesh,
    const preciceAdapter::FieldConfig& fieldConfig)
: vectorField_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(fieldConfig.solver_name))),
  mesh_(mesh),
  fieldConfig_(fieldConfig)
{
    dataType_ = vector;
}

void preciceAdapter::Generic::VectorFieldCoupler::initialize()
{
    // In the Generic module, for vector fields only the value operation is supported for now, i.e., cannot write gradients.
    if (fieldConfig_.operation == "gradient" || fieldConfig_.operation == "surface-normal-gradient")
    {
        adapterInfo("Generic module: The gradient operation is not yet supported for vector fields.", "error");
    }
}

std::size_t preciceAdapter::Generic::VectorFieldCoupler::write(double* buffer, bool meshConnectivity, const unsigned int dim)
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

void preciceAdapter::Generic::VectorFieldCoupler::read(double* buffer, const unsigned int dim)
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

        auto& bc = vectorField_->boundaryFieldRef()[patchID];

        if (isA<fixedValueFvPatchVectorField>(bc))
        {
            auto& boundaryPatch = refCast<fixedValueFvPatchVectorField>(bc);
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
        else if (isA<fixedGradientFvPatchVectorField>(bc))
        {
            auto& boundaryPatch = refCast<fixedGradientFvPatchVectorField>(bc);
            forAll(boundaryPatch, i)
            {
                boundaryPatch.gradient()[i].x() = buffer[bufferIndex++];
                boundaryPatch.gradient()[i].y() = buffer[bufferIndex++];

                if (dim == 3)
                {
                    boundaryPatch.gradient()[i].z() = buffer[bufferIndex++];
                }
            }
        }
        else
        {
            adapterInfo("Generic module: Unsupported boundary condition type " + bc.type(), "error");
        }
    }
}

bool preciceAdapter::Generic::VectorFieldCoupler::isLocationTypeSupported(const bool meshConnectivity) const
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

std::string preciceAdapter::Generic::VectorFieldCoupler::getDataName() const
{
    return fieldConfig_.name;
}
