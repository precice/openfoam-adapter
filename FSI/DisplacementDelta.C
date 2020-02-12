#include "DisplacementDelta.H"

using namespace Foam;

preciceAdapter::FSI::DisplacementDelta::DisplacementDelta
(
    const Foam::fvMesh& mesh,
    const std::string nameDPointDisplacement
)
:
DpointDisplacement_(
    const_cast<pointVectorField*>
    (
        &mesh.lookupObject<pointVectorField>(nameDPointDisplacement)
    )
)
{
    dataType_ = vector;
}

void preciceAdapter::FSI::DisplacementDelta::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;
    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);        
                
        // Get the displacement on the patch
        fixedValuePointPatchVectorField& DpointDisplacementPatch =
            refCast<fixedValuePointPatchVectorField>
            (
                DpointDisplacement_->boundaryFieldRef()[patchID]
            );            
                
        // Write the displacements to the preCICE buffer
        // For every cell of the patch
        forAll(DpointDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Copy the displacement into the buffer
            // x-dimension
            buffer[bufferIndex++]
            = 
            DpointDisplacementPatch[i][0];

            // y-dimension
            buffer[bufferIndex++]
            =
            DpointDisplacementPatch[i][1];

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                =
                DpointDisplacementPatch[i][2];
        }       
    }
}

// return the displacement to use later in the velocity?
void preciceAdapter::FSI::DisplacementDelta::read(double * buffer, const unsigned int dim)
{
    // For every element in the buffer
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the displacement on the patch
        fixedValuePointPatchVectorField& pointDisplacementPatch =
            refCast<fixedValuePointPatchVectorField>
            (
                DpointDisplacement_->boundaryFieldRef()[patchID]
            );

        // For every cell of the patch
        forAll(DpointDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Set the displacementDeltas to the received one
            pointDisplacementPatch[i][0] = buffer[bufferIndex++];
            pointDisplacementPatch[i][1] = buffer[bufferIndex++];
            if(dim == 3)
                pointDisplacementPatch[i][2] = buffer[bufferIndex++];
        }      
    }
}
