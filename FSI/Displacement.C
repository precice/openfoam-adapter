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
)
{
    dataType_ = vector;
}

void preciceAdapter::FSI::Displacement::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
        
    int bufferIndex = 0;
    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);
        
        // Get the displacement on the patch
        fixedValuePointPatchVectorField& pointDisplacementFluidPatch =
            refCast<fixedValuePointPatchVectorField>
            (
                pointDisplacement_->boundaryFieldRef()[patchID]
            );
        
        // Write the displacements to the preCICE buffer
        // For every cell of the patch
        forAll(pointDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Copy the dispalcement into the buffer
            // x-dimension
            buffer[bufferIndex++]
            = 
            pointDisplacementFluidPatch[i][0];

            // y-dimension
            buffer[bufferIndex++]
            =
            pointDisplacementFluidPatch[i][1];

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                =
                pointDisplacementFluidPatch[i][2];
        }
    }
}

// return the displacement to use later in the velocity?
void preciceAdapter::FSI::Displacement::read(double * buffer, const unsigned int dim)
{
    // For every element in the buffer
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the displacement on the patch
        fixedValuePointPatchVectorField& pointDisplacementFluidPatch =
            refCast<fixedValuePointPatchVectorField>
            (
                pointDisplacement_->boundaryFieldRef()[patchID]
            );

        // For every cell of the patch
        forAll(pointDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Set the displacement to the received one
            pointDisplacementFluidPatch[i][0] = buffer[bufferIndex++];
            pointDisplacementFluidPatch[i][1] = buffer[bufferIndex++];
            if(dim == 3)
                pointDisplacementFluidPatch[i][2] = buffer[bufferIndex++];
        }
    }
}
