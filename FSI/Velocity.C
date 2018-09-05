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
runTime_(runTime),
Displ_(),
Velocity_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameVelocity)
    )
)
{
    dataType_ = vector;

    // Initialize the Displacement arrays to zero vectors
    // Displ_ = new vectorField(numDataLocations_, Foam::vector::zero);
    // DisplOld_ = new vectorField(Velocity_->boundaryFieldRef().size()*3, Foam::vector::zero);
    Displ_    = new double[Velocity_->boundaryFieldRef().size()*3];
    DisplOld_ = new double[Velocity_->boundaryFieldRef().size()*3];
}



void preciceAdapter::FSI::Velocity::write(double * buffer)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE writeBlockVectorData() implementation.
    */
    FatalErrorInFunction
        << "Writing velocities is not supported."
        << exit(FatalError);
}

void preciceAdapter::FSI::Velocity::read(double * buffer)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE readBlockVectorData() implementation.
    */


    // TODO: Check this
    DisplOld_ = Displ_;
    *Displ_ = *buffer;      // write for next timestep

    // For every element in the buffer
    // TODO: Check if this works correctly with multiple patches
    
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);


        // Get the pointMotionU
        volVectorField& velocityPatch =
           refCast<volVectorField>
            (
                Velocity_->boundaryFieldRef()[patchID]
            );

        // TODO: Extend this to multiple patches
        // fixedValuePointPatchVectorField& motionUFluidPatch =
        //     refCast<fixedValuePointPatchVectorField>
        //     (
        //         motionU.boundaryFieldRef()[patchIDs_[0]]
        //     );
        // For every cell of the patch
        
        forAll(Velocity_->boundaryFieldRef()[patchID], i)
        {
            // Set the buffer as the pointdisplacement value
            velocityPatch[i][0] = (Displ_[bufferIndex] - DisplOld_[bufferIndex]) / runTime_.deltaT().value();
            bufferIndex++;
            velocityPatch[i][1] = (Displ_[bufferIndex] - DisplOld_[bufferIndex]) / runTime_.deltaT().value();
            bufferIndex++;
            velocityPatch[i][2] = (Displ_[bufferIndex] - DisplOld_[bufferIndex]) / runTime_.deltaT().value();
            bufferIndex++;
        }
    }
}
