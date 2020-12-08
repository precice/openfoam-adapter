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
    // For every element in the buffer
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const volVectorField &tmp_field = mesh_.lookupObject<volVectorField>("cellDisplacement");
        vectorField           cellDisplacement(tmp_field.boundaryField()[patchID]);
        forAll(cellDisplacement, i)
        {
            // Set the displacement to the received one
            cellDisplacement[i][0] = buffer[bufferIndex++];
            cellDisplacement[i][1] = buffer[bufferIndex++];
            if(dim ==3)
                cellDisplacement[i][2] = buffer[bufferIndex++];
        }

        //Interpolate from centers to nodes
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);
        vectorField pointDisplacement
        (
            patchInterpolator.faceToPointInterpolate(cellDisplacement)
        );

        // Get the displacement on the patch
        fixedValuePointPatchVectorField& pointDisplacementFluidPatch
        (
            refCast<fixedValuePointPatchVectorField>
            (
                pointDisplacement_->boundaryFieldRef()[patchID]
            )
        );

        // For every cell of the patch
        forAll(pointDisplacement_->boundaryFieldRef()[patchID], i)
        {
            // Set the displacement to the received one
            pointDisplacementFluidPatch[i][0] = pointDisplacement[i][0];
            pointDisplacementFluidPatch[i][1] = pointDisplacement[i][1];
            if(dim ==3)
                pointDisplacementFluidPatch[i][2] = pointDisplacement[i][2];
        }
    }
}
