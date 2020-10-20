#include "DisplacementDelta.H"

using namespace Foam;

preciceAdapter::FSI::DisplacementDelta::DisplacementDelta
(
    const Foam::fvMesh& mesh,
    const std::string namePointDisplacement
)
:
CouplingDataUser(DT_Vector),
pointDisplacement_(mesh.lookupObjectRef<pointVectorField>(namePointDisplacement))
{}

void preciceAdapter::FSI::DisplacementDelta::write(std::vector<double> &dataBuffer, bool meshConnectivity, const unsigned int dim)
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
void preciceAdapter::FSI::DisplacementDelta::read(const std::vector<double> &dataBuffer, const unsigned int dim)
{
    // For every element in the buffer
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++)
    {
        const auto patchID = patchIDs_.at(j);

        // Get the displacement on the patch
        fixedValuePointPatchVectorField& pointDisplacementFluidPatch
        (
            refCast<fixedValuePointPatchVectorField>
            (
                pointDisplacement_.boundaryFieldRef()[patchID]
            )
        );

        // For every cell of the patch
        forAll(pointDisplacement_.boundaryFieldRef()[patchID], i)
        {
            // Add the received delta to the actual displacement
            pointDisplacementFluidPatch[i][0] += dataBuffer[bufferIndex++];
            pointDisplacementFluidPatch[i][1] += dataBuffer[bufferIndex++];
            if(dim == 3)
                pointDisplacementFluidPatch[i][2] += dataBuffer[bufferIndex++];
        }
    }
}
