#include "Velocity.H"

using namespace Foam;


preciceAdapter::FSI::Velocity::Velocity
(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime,
    const std::string nameVelocity
    /* TODO: We should add any required field names here.
    /  They would need to be vector fields.
    /  See CHT/Temperature.C for details.
    */
)
:
mesh_(mesh),
runTime_(runTime),
velocity_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameVelocity)
    )
)
{
    dataType_ = vector;

    // Initialize the Displacement arrays to zero vectors. This is still quite ugly
    faceDisplacement_ = new vectorField(velocity_->boundaryFieldRef().size()*3, Foam::vector::zero);
    faceDisplacementOld_ = new vectorField(velocity_->boundaryFieldRef().size()*3, Foam::vector::zero);
}



void preciceAdapter::FSI::Velocity::write(double * buffer)
{
    /* TODO: Implement
    * FOR NOW ONLY WORKS IF THE DISPLACEMENT FIELD IS ALREADY UPDATED. 
    * Make this function not dependent on the buffer, but rather on the faceDisplacement
    * This can be a function in the displacement.C
    * Create velocity interpolation
    */
    FatalErrorInFunction
        << "Writing velocities is not supported."
        << exit(FatalError);
}

void preciceAdapter::FSI::Velocity::read(double * buffer)
{
    /* TODO: Implement
    * FOR NOW ONLY WORKS IF THE DISPLACEMENT FIELD IS ALREADY UPDATED. 
    * Make this function not dependent on the buffer, but rather on the faceDisplacement
    * This can be a function in the displacement.C
    * Create velocity interpolation
    */

    *faceDisplacementOld_ = *faceDisplacement_; 

    // TODO: Check this
    // DisplOld_ = Displ_;
    // *Displ_ = *buffer;      // write for next timestep

    // For every element in the buffer
    // TODO: Check if this works correctly with multiple patches
    
    // int bufferIndex = 0;

    // Use the following function
    //tmp<Field<Type>> PrimitivePatchInterpolation<Patch>::pointToFaceInterpolate

        // Get the pointdisplacement (HACK)
    pointVectorField& pointDisplacement_ =
        const_cast<pointVectorField&>
        (
            mesh_.lookupObject<pointVectorField>("pointDisplacement")
        );

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);


        // Get the pointMotionU
        volVectorField& velocityPatch =
           refCast<volVectorField>
            (
                velocity_->boundaryFieldRef()[patchID]
            );

        fixedValuePointPatchVectorField& pointDisplacementPatch = 
            refCast<fixedValuePointPatchVectorField>
            (
                pointDisplacement_.boundaryFieldRef()[patchID]
            );


        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID] );

        faceDisplacement_[patchID]
            =
            patchInterpolator.pointToFaceInterpolate(pointDisplacementPatch);


        // This displacement only works without subcycling
        // For subcycling this function must be called every timestep with some approximation. 
        // loop over the cells of the patch
        forAll(velocity_->boundaryFieldRef()[patchID], i)
        {
            velocityPatch[i][0] = (faceDisplacement_[patchID][i][0] - faceDisplacementOld_[patchID][i][0]) / runTime_.deltaT().value();
            velocityPatch[i][1] = (faceDisplacement_[patchID][i][1] - faceDisplacementOld_[patchID][i][1]) / runTime_.deltaT().value();
            velocityPatch[i][2] = (faceDisplacement_[patchID][i][2] - faceDisplacementOld_[patchID][i][2]) / runTime_.deltaT().value();
        }
        



        // Field<fixedValuePointPatchVectorField> velocityPatch(ppi().pointToFaceInterpolate(pointDisplacementFluidPatch));
       
        // velocity_->boundaryFieldRef()[patchID]
        //     =
        //     PrimitivePatchInterpolation<primitivePatch>::faceToPointInterpolate
        //         (
        //             const <fixedValuePointPatchVectorField> 
        //              pointDisplacementFluidPatch
        //         );
            // tmp<pointDisplacementFluidPatch<fixedValuePointPatchVectorField>> 


        // velocity_->boundaryFieldRef()[patchID]
        //     (
        //         patchInterpolator().pointToFaceInterpolate
        //         (            (
        //             pointDisplacement_.boundaryFieldRef()[patchID]
        //         )
        //      );


            // Set the buffer as the pointdisplacement value
            // velocityPatch[i][0] = (Displ_[bufferIndex] - DisplOld_[bufferIndex]) 
            // / runTime_.deltaT().value();
            // bufferIndex++;
            // velocityPatch[i][1] = (Displ_[bufferIndex] - DisplOld_[bufferIndex]) / runTime_.deltaT().value();
            // bufferIndex++;
            // velocityPatch[i][2] = (Displ_[bufferIndex] - DisplOld_[bufferIndex]) / runTime_.deltaT().value();
            // bufferIndex++;
        // }
    

    }
}
