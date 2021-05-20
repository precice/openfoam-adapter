#include "Generic_volVectorField.H"

using namespace Foam;

preciceAdapter::Momentum::Generic_volVectorField::Generic_volVectorField
(
    const Foam::fvMesh& mesh,
    const std::string nameGeneric_volVectorField
)
:
generic_volVectorField_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameGeneric_volVectorField)
    )
)
{
    dataType_ = vector;
}

void preciceAdapter::Momentum::Generic_volVectorField::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
  int bufferIndex = 0;

  forAll(generic_volVectorField_->internalField(), k)
  {
      // x-dimension
      buffer[bufferIndex++]
      =
      generic_volVectorField_->internalField()[k].x();

      // y-dimension
      buffer[bufferIndex++]
      =
      generic_volVectorField_->internalField()[k].y();

      if(dim == 3)
          // z-dimension
          buffer[bufferIndex++]
          =
          generic_volVectorField_->internalField()[k].z();
  }

  // For every boundary patch of the interface
  for (uint j = 0; j < patchIDs_.size(); j++)
  {
      int patchID = patchIDs_.at(j);

      fvPatchVectorField & UPatch =
          refCast<fvPatchVectorField>
              (
                  generic_volVectorField_->boundaryFieldRef()[patchID]
              );

      // Write the field to the preCICE buffer
      // For every cell of the patch
      forAll(generic_volVectorField_->boundaryFieldRef()[patchID], i)
      {
          // Copy the field into the buffer
          // x-dimension
          buffer[bufferIndex++]
          =
          UPatch[i].x();

          // y-dimension
          buffer[bufferIndex++]
          =
          UPatch[i].y();

          if(dim == 3)
              // z-dimension
              buffer[bufferIndex++]
              =
              UPatch[i].z();
      }
  }
}

void preciceAdapter::Momentum::Generic_volVectorField::read(double * buffer, const unsigned int dim)
{
 // For every element in the buffer
  /*int bufferIndex = 0;

  // For every boundary patch of the interface
  for (uint j = 0; j < patchIDs_.size(); j++)
  {
      int patchID = patchIDs_.at(j);

      // Get the field on the patch
      fixedValuePointPatchVectorField& pointGeneric_volVectorPatch =
          refCast<fvPatchVectorField>
          (
              generic_volVectorField_->boundaryFieldRef()[patchID]
          );

      // For every cell of the patch
      forAll(generic_volVectorField_->boundaryFieldRef()[patchID], i)
      {
          // Set the field to the received one
          pointGeneric_volVectorPatch[i][0] = buffer[bufferIndex++];
          pointGeneric_volVectorPatch[i][1] = buffer[bufferIndex++];
          if(dim ==3)
              pointGeneric_volVectorPatch[i][2] = buffer[bufferIndex++];
      }
  }*/
}
