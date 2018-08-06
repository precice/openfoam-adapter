#include "Displacement.H"

using namespace Foam;

preciceAdapter::FSI::Displacement::Displacement
(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime
    /* TODO: We should add any required field names here.
    /  They would need to be vector fields.
    /  See CHT/Temperature.C for details.
    */
)
:
mesh_(mesh),
runTime_(runTime)
/* TODO: We probably need to initialize some fields here,
/  see CHT/Temperature.C.
*/
{
    dataType_ = vector;

    // Count the data locations for all the patches
    numDataLocations_ = 0;
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        numDataLocations_ +=
            mesh.boundaryMesh()[patchIDs_.at(j)].localPoints().size();
    }

    // Initialize displacements arrays
    vectorField Displ_(3*numDataLocations_, Foam::vector::zero);
    vectorField DisplOld_(3*numDataLocations_, Foam::vector::zero);
}

void preciceAdapter::FSI::Displacement::write(double * buffer)
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

void preciceAdapter::FSI::Displacement::read(double * buffer)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE readBlockVectorData() implementation.
    */

    // Store the previous displacements
    DisplOld_ = Displ_;

    // For every element in the buffer
    // TODO: Check if this works correctly with multiple patches
    int bufferIndex = 0;

    // For every point (data location)
    for (int i = 0; i < numDataLocations_; i++)
    {
        // TODO: Is there a x,y,z notation?
        Displ_[i][0] = buffer[bufferIndex++];
        Displ_[i][1] = buffer[bufferIndex++];
        Displ_[i][2] = buffer[bufferIndex++];
    }

    // Get the pointMotionU
    pointVectorField& motionU =
        const_cast<pointVectorField&>
        (
            mesh_.lookupObject<pointVectorField>("pointMotionU")
        );

    // TODO: Extend this to multiple patches
    fixedValuePointPatchVectorField& motionUFluidPatch =
        refCast<fixedValuePointPatchVectorField>
        (
            motionU.boundaryFieldRef()[patchIDs_[0]]
        );

    motionUFluidPatch ==
        (Displ_ - DisplOld_) /  runTime_.deltaT().value();

}
