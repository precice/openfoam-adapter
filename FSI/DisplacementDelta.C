#include "DisplacementDelta.H"

using namespace Foam;

preciceAdapter::FSI::DisplacementDelta::DisplacementDelta
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

void preciceAdapter::FSI::DisplacementDelta::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE writeBlockVectorData() implementation.
    */
    FatalErrorInFunction
        << "Writing displacementDeltas is not supported."
        << exit(FatalError);
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

        const volVectorField &tmp_field = mesh_.lookupObject<volVectorField>("cellDisplacement");
        vectorField           cellDisplacement(tmp_field.boundaryField()[patchID]);
        forAll(cellDisplacement, i)
        {
            // Set the displacement to the received one
            cellDisplacement[i][0] += buffer[bufferIndex++];
            cellDisplacement[i][1] += buffer[bufferIndex++];
            if(dim ==3)
                cellDisplacement[i][2] += buffer[bufferIndex++];
        }

        // Get the displacement on the patch
        vectorField& pointDisplacementFluidPatch
        (
            refCast<vectorField>
            (
                pointDisplacement_->boundaryFieldRef()[patchID]
            )
        );

        //Interpolate from centers to nodes
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);
        pointDisplacementFluidPatch =
             patchInterpolator.faceToPointInterpolate(cellDisplacement);
    }
}
