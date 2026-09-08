#include "DisplacementDelta.H"

using namespace Foam;

preciceAdapter::FSI::DisplacementDelta::DisplacementDelta(
    const Foam::fvMesh& mesh,
    const std::string namePointDisplacement,
    const std::string nameCellDisplacement)
: pointDisplacement_(
    const_cast<pointVectorField*>(
        &mesh.lookupObject<pointVectorField>(namePointDisplacement))),
  cellDisplacement_(
      const_cast<volVectorField*>(
          &mesh.lookupObject<volVectorField>(nameCellDisplacement))),
  mesh_(mesh)
{
    dataType_ = vector;
}

// We cannot do this step in the constructor by design of the adapter since the information of the CouplingDataUser is
// defined later. Hence, we call this method after the CouplingDaaUser has been configured
void preciceAdapter::FSI::DisplacementDelta::initialize()
{
    // Initialize appropriate objects for each interface patch, namely the volField and the interpolation object
    // this is only necessary for face based FSI
    if (this->locationType_ == LocationType::faceCenters)
    {
        for (unsigned int j = 0; j < patchIDs_.size(); ++j)
        {
            const unsigned int patchID = patchIDs_.at(j);
            interpolationObjects_.emplace_back(new primitivePatchInterpolation(mesh_.boundaryMesh()[patchID]));
        }
    }
}


std::size_t preciceAdapter::FSI::DisplacementDelta::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    // Copy the displacement increment over the current time window from
    // OpenFOAM to the buffer. The reference state is the stored old-time
    // field: the adapter checkpoints oldTime() (see Adapter::readCheckpoint),
    // so re-iterating an implicit time window does not shift the reference.

    int bufferIndex = 0;
    if (this->locationType_ == LocationType::faceCenters)
    {
        // For every boundary patch of the interface
        for (const label patchID : patchIDs_)
        {
            const vectorField& displacement =
                cellDisplacement_->boundaryField()[patchID];
            const vectorField& displacementOld =
                cellDisplacement_->oldTime().boundaryField()[patchID];

            // Write the displacement increment to the preCICE buffer
            // For every cell of the patch
            forAll(displacement, i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    buffer[bufferIndex++] =
                        displacement[i][d] - displacementOld[i][d];
            }
        }
    }
    else if (this->locationType_ == LocationType::faceNodes)
    {
        DEBUG(adapterInfo(
            "Please be aware of issues with using 'locationType faceNodes' "
            "in parallel. \n"
            "See https://github.com/precice/openfoam-adapter/issues/153.",
            "warning"));

        // For every boundary patch of the interface
        for (const label patchID : patchIDs_)
        {
            const auto& displacement = pointDisplacement_->internalField();
            const auto& displacementOld =
                pointDisplacement_->oldTime().internalField();

            // Write the displacement increment to the preCICE buffer
            // For every node of the patch
            forAll(pointDisplacement_->boundaryField()[patchID], i)
            {
                const labelList& meshPoints =
                    mesh_.boundaryMesh()[patchID].meshPoints();

                for (unsigned int d = 0; d < dim; ++d)
                    buffer[bufferIndex++] =
                        displacement[meshPoints[i]][d]
                        - displacementOld[meshPoints[i]][d];
            }
        }
    }
    return bufferIndex;
}

// return the displacement to use later in the velocity?
void preciceAdapter::FSI::DisplacementDelta::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;
    for (unsigned int j = 0; j < patchIDs_.size(); j++)
    {
        // Get the ID of the current patch
        const unsigned int patchID = patchIDs_.at(j);

        if (this->locationType_ == LocationType::faceCenters)
        {

            // the boundaryCellDisplacement is a vector and ordered according to the iterator j
            // and not according to the patchID
            // First, copy the buffer data into the center based vectorFields on each interface patch
            // For DisplacementDelta, set absolute values here and sum the interpolated values up to the point field
            // since the temporary field in this class is not reloaded in the implicit coupling
            forAll(cellDisplacement_->boundaryField()[patchID], i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    cellDisplacement_->boundaryFieldRef()[patchID][i][d] = buffer[bufferIndex++];
            }
            // Get a reference to the displacement on the point patch in order to overwrite it
            vectorField& pointDisplacementFluidPatch(
                refCast<vectorField>(
                    pointDisplacement_->boundaryFieldRef()[patchID]));

            // Overwrite the node based patch using the interpolation objects and the cell based vector field
            // Afterwards, continue as usual
            pointDisplacementFluidPatch += interpolationObjects_[j]->faceToPointInterpolate(cellDisplacement_->boundaryField()[patchID]);
        }
        else if (this->locationType_ == LocationType::faceNodes)
        {

            // Get the displacement on the patch
            fixedValuePointPatchVectorField& pointDisplacementFluidPatch(
                refCast<fixedValuePointPatchVectorField>(
                    pointDisplacement_->boundaryFieldRef()[patchID]));

            // Overwrite the nodes on the interface directly
            forAll(pointDisplacement_->boundaryFieldRef()[patchID], i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    pointDisplacementFluidPatch[i][d] += buffer[bufferIndex++];
            }
        }
    }
}

bool preciceAdapter::FSI::DisplacementDelta::isLocationTypeSupported(const bool meshConnectivity) const
{
    // Solid solver *could* allow connectivity for writing displacement
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceNodes);
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::faceNodes);
    }
}

std::string preciceAdapter::FSI::DisplacementDelta::getDataName() const
{
    return "DisplacementDelta";
}
