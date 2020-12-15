#include "Displacement.H"

using namespace Foam;

preciceAdapter::FSI::Displacement::Displacement
(
    const Foam::fvMesh& mesh,
    const std::string namePointDisplacement
)
:
pointDisplacement_(
    const_cast<pointVectorField*>
    (
        &mesh.lookupObject<pointVectorField>(namePointDisplacement)
    )
),
mesh_(mesh)
{
    dataType_ = vector;
}

// We cannot do this steps in the constructor by design of the adapter since the information of the CouplingDataUser is
// defined later. Hence, we call this method after the CouplingDaaUser has been configured
void preciceAdapter::FSI::Displacement::initialize()
{
  // We use the cellDisplacement in order to copy the interface boundary fields
  const volVectorField &cellDisplacement_ =
      mesh_.lookupObject<volVectorField>("cellDisplacement");

  // Initialize appropriate objects for each interface patch, namely the volField and the interpolation object
  // this is only necessary for face based FSI
  if (this->locationsType_ == "faceCenters" || this->locationsType_ == "faceCentres")
    for (unsigned int j = 0; j < patchIDs_.size(); j++) {
      const unsigned int patchID = patchIDs_.at(j);
      boundaryCellDisplacement_.emplace_back(cellDisplacement_.boundaryField()[patchID]);
      interpolationObjects_.emplace_back(new primitivePatchInterpolation(mesh_.boundaryMesh()[patchID]));
    }
}



void preciceAdapter::FSI::Displacement::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE writeBlockVectorData() implementation.
    */
    FatalErrorInFunction
        << "Writing displacements is not supported."
        << exit(FatalError);
}


// return the displacement to use later in the velocity?
void preciceAdapter::FSI::Displacement::read(double * buffer, const unsigned int dim)
{
    for (unsigned int j = 0; j < patchIDs_.size(); j++)
    {
        // Get the ID of the current patch
        const unsigned int patchID = patchIDs_.at(j);

        if (this->locationsType_ == "faceCenters" || this->locationsType_ == "faceCentres") {

          // the boundaryCellDisplacement is a vector and ordered according to the iterator j
          // and not according to the patchID
          // First, copy the buffer data into the center based vectorFields on each interface patch
          for (int i = 0; i < boundaryCellDisplacement_[j].size(); ++i) {
            for (unsigned int d = 0; d < dim; ++d)
              boundaryCellDisplacement_[j][i][d] = buffer[i * dim + d];
          }
          // Get a reference to the displacement on the point patch in order to overwrite it
          vectorField &pointDisplacementFluidPatch(
              refCast<vectorField>(
                  pointDisplacement_->boundaryFieldRef()[patchID]));

          // Overwrite the node based patch using the interpolation objects and the cell based vector field
          // Afterwards, continue as usual
          pointDisplacementFluidPatch = interpolationObjects_[j]->faceToPointInterpolate(boundaryCellDisplacement_[j]);

        } else if (this->locationsType_ == "faceNodes") {

          // Get the displacement on the patch
          fixedValuePointPatchVectorField &pointDisplacementFluidPatch(
              refCast<fixedValuePointPatchVectorField>(
                  pointDisplacement_->boundaryFieldRef()[patchID]));

          // Overwrite the nodes on the interface directly
          for (int i = 0; i < pointDisplacement_->boundaryFieldRef()[patchID].size(); ++i) {
            for (unsigned int d = 0; d < dim; ++d)
              pointDisplacementFluidPatch[i][d] = buffer[i * dim + d];
          }
        }
    }
}
