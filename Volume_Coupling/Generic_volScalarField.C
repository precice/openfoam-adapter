#include "Generic_volScalarField.H"

using namespace Foam;

preciceAdapter::Momentum::Generic_volScalarField::Generic_volScalarField
(
    const Foam::fvMesh& mesh,
    const std::string nameGeneric_volScalarField
)
:
generic_volScalarField_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameGeneric_volScalarField)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::Momentum::Generic_volScalarField::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
  int bufferIndex = 0;

  forAll(generic_volScalarField_->internalField(), k)
  {
      buffer[bufferIndex++]
      =
      generic_volScalarField_->internalField()[k];
  }

  // For every boundary patch of the interface
  for (uint j = 0; j < patchIDs_.size(); j++)
  {
      int patchID = patchIDs_.at(j);

      fvPatchScalarField & RPatch =
          refCast<fvPatchScalarField>
              (
                  generic_volScalarField_->boundaryFieldRef()[patchID]
              );

      // Write the field to the preCICE buffer
      // For every cell of the patch
      forAll(generic_volScalarField_->boundaryFieldRef()[patchID], i)
      {
          buffer[bufferIndex++]
          =
          RPatch[i];
      }
  }
}

void preciceAdapter::Momentum::Generic_volScalarField::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // for every internal cell
    forAll(generic_volScalarField_->ref(), k)
    {
        generic_volScalarField_->ref()[k]
        =
        buffer[bufferIndex++] ;
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the boundary field of the field on the patch
        fvPatchScalarField & PPatch =
            refCast<fvPatchScalarField>
            (
                generic_volScalarField_->boundaryFieldRef()[patchID]
            );

        // For every cell of the patch
        forAll(generic_volScalarField_->boundaryFieldRef()[patchID], i)
        {
            // Copy the internal field into the buffer
            PPatch[i]
            =
            buffer[bufferIndex++];
        }
    }
}
