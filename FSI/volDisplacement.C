#include "volDisplacement.H"

using namespace Foam;

preciceAdapter::FSI::volDisplacement::volDisplacement
(
    const Foam::fvMesh& mesh,
    const std::string nameCellDisplacement
)
:
cellDisplacement_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameCellDisplacement)
    )
)
{
    dataType_ = vector;
}

void preciceAdapter::FSI::volDisplacement::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;
    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);
        
        // Get the displacement on the patch
        fixedValueFvPatchVectorField& cellDisplacementFluidPatch =
            refCast<fixedValueFvPatchVectorField>
            (
                cellDisplacement_->boundaryFieldRef()[patchID]
            );
        
        // Write the displacements to the preCICE buffer
        // For every cell of the patch
        forAll(cellDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Copy the dispalcement into the buffer
            // x-dimension
            buffer[bufferIndex++]
            = 
            cellDisplacementFluidPatch[i][0];

            // y-dimension
            buffer[bufferIndex++]
            =
            cellDisplacementFluidPatch[i][1];

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                =
                cellDisplacementFluidPatch[i][2];
        }
    }
}

// return the displacement to use later in the velocity?
void preciceAdapter::FSI::volDisplacement::read(double * buffer, const unsigned int dim)
{
    // For every element in the buffer
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the displacement on the patch
        fixedValueFvPatchVectorField& cellDisplacementFluidPatch =
            refCast<fixedValueFvPatchVectorField>
            (
                cellDisplacement_->boundaryFieldRef()[patchID]
            );

        // For every cell of the patch
        forAll(cellDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Set the displacement to the received one
            cellDisplacementFluidPatch[i][0] = buffer[bufferIndex++];
            cellDisplacementFluidPatch[i][1] = buffer[bufferIndex++];
            if(dim == 3)
                cellDisplacementFluidPatch[i][2] = buffer[bufferIndex++];
        }
    }
}
