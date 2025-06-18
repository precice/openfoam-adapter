#include "Displacement.H"

using namespace Foam;

preciceAdapter::FSI::Displacement::Displacement(
    const Foam::fvMesh& mesh,
    const std::string namePointDisplacement,
    const std::string nameCellDisplacement)
: pointDisplacement_(
    namePointDisplacement == "unused"
        ? nullptr
        : const_cast<pointVectorField*>(
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
void preciceAdapter::FSI::Displacement::initialize()
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


std::size_t preciceAdapter::FSI::Displacement::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    Foam::scalar maxDispMag = 0;
    Foam::scalar sumDispMag = 0;

    if (this->locationType_ == LocationType::faceCenters)
    {
        for (const label patchID : patchIDs_)
        {
            const fvPatchVectorField& dispPatch = cellDisplacement_->boundaryField()[patchID];
            forAll(dispPatch, i)
            {
                Foam::scalar magD = mag(dispPatch[i]);
                sumDispMag += magD;
                if (magD > maxDispMag)
                {
                    maxDispMag = magD;
                }
            }
        }
    }
    else if (this->locationType_ == LocationType::faceNodes)
    {
        for (const label patchID : patchIDs_)
        {
            const labelList& meshPoints = mesh_.boundaryMesh()[patchID].meshPoints();
            forAll(pointDisplacement_->boundaryField()[patchID], i)
            {
                const Foam::vector& disp = pointDisplacement_->internalField()[meshPoints[i]];
                Foam::scalar magD = mag(disp);
                sumDispMag += magD;
                if (magD > maxDispMag)
                {
                    maxDispMag = magD;
                }
            }
        }
    }

    DEBUG(adapterInfo(
        "PRECICE_DEBUG_DISPLACEMENT_WRITE_MAX_MAG TIME=" + std::to_string(mesh_.time().value()) + " VALUE=" + std::to_string(maxDispMag)));
    DEBUG(adapterInfo(
        "PRECICE_DEBUG_DISPLACEMENT_WRITE_SUM_MAG TIME=" + std::to_string(mesh_.time().value()) + " VALUE=" + std::to_string(sumDispMag)));

    int bufferIndex = 0;
    if (this->locationType_ == LocationType::faceCenters)
    {
        // For every boundary patch of the interface
        for (const label patchID : patchIDs_)
        {
            // Write the displacement to the preCICE buffer
            // For every cell of the patch
            forAll(cellDisplacement_->boundaryField()[patchID], i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    buffer[bufferIndex++] =
                        cellDisplacement_->boundaryField()[patchID][i][d];
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
            // Write the displacement to the preCICE buffer
            // For every cell of the patch
            forAll(pointDisplacement_->boundaryField()[patchID], i)
            {
                const labelList& meshPoints =
                    mesh_.boundaryMesh()[patchID].meshPoints();

                for (unsigned int d = 0; d < dim; ++d)
                    buffer[bufferIndex++] =
                        pointDisplacement_->internalField()[meshPoints[i]][d];
            }
        }
    }
    return bufferIndex;
}


// return the displacement to use later in the velocity?
void preciceAdapter::FSI::Displacement::read(double* buffer, const unsigned int dim)
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
            forAll(cellDisplacement_->boundaryField()[patchID], i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    cellDisplacement_->boundaryFieldRef()[patchID][i][d] = buffer[bufferIndex++];
            }

            if (pointDisplacement_ != nullptr)
            {
                // Get a reference to the displacement on the point patch in order to overwrite it
                vectorField& pointDisplacementFluidPatch(
                    refCast<vectorField>(
                        pointDisplacement_->boundaryFieldRef()[patchID]));

                // Overwrite the node based patch using the interpolation objects and the cell based vector field
                // Afterwards, continue as usual
                pointDisplacementFluidPatch = interpolationObjects_[j]->faceToPointInterpolate(cellDisplacement_->boundaryField()[patchID]);
            }
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
                    pointDisplacementFluidPatch[i][d] = buffer[bufferIndex++];
            }
        }
    }

    Foam::scalar maxDispMag = 0;
    Foam::scalar sumDispMag = 0;

    if (this->locationType_ == LocationType::faceCenters)
    {
        for (const label patchID : patchIDs_)
        {
            const fvPatchVectorField& dispPatch = cellDisplacement_->boundaryField()[patchID];
            forAll(dispPatch, i)
            {
                Foam::scalar magD = mag(dispPatch[i]);
                sumDispMag += magD;
                if (magD > maxDispMag)
                {
                    maxDispMag = magD;
                }
            }
        }
    }
    else if (this->locationType_ == LocationType::faceNodes)
    {
        for (const label patchID : patchIDs_)
        {
            const fixedValuePointPatchVectorField& dispPatch =
                refCast<const fixedValuePointPatchVectorField>(
                    pointDisplacement_->boundaryField()[patchID]);
            forAll(dispPatch, i)
            {
                Foam::scalar magD = mag(dispPatch[i]);
                sumDispMag += magD;
                if (magD > maxDispMag)
                {
                    maxDispMag = magD;
                }
            }
        }
    }

    DEBUG(adapterInfo(
        "PRECICE_DEBUG_DISPLACEMENT_READ_MAX_MAG TIME=" + std::to_string(mesh_.time().value()) + " VALUE=" + std::to_string(maxDispMag)));
    DEBUG(adapterInfo(
        "PRECICE_DEBUG_DISPLACEMENT_READ_SUM_MAG TIME=" + std::to_string(mesh_.time().value()) + " VALUE=" + std::to_string(sumDispMag)));
}

bool preciceAdapter::FSI::Displacement::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::faceNodes);
}

std::string preciceAdapter::FSI::Displacement::getDataName() const
{
    return "Displacement";
}
