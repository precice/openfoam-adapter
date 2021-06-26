#include "Fluid_Velocity.H"

using namespace Foam;

preciceAdapter::Volume_Coupling::Fluid_Velocity::Fluid_Velocity
(
    const Foam::fvMesh& mesh,
    const std::string nameU
)
:

Fluid_Velocity_(
        const_cast<volVectorField*>
      (
            &mesh.lookupObject<volVectorField>(nameU)
      )
 )
{
    dataType_ = vector;
}




void preciceAdapter::Volume_Coupling::Fluid_Velocity::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    // Compute forces. See the Forces function object.
    // Normal vectors on the boundary, multiplied with the face areas

    int bufferIndex = 0;

    forAll(Fluid_Velocity_->internalField(), k)
    {
        buffer[bufferIndex++]
        =
        Fluid_Velocity_->internalField()[k].x();

        // y-dimension
        buffer[bufferIndex++]
        =
        Fluid_Velocity_->internalField()[k].y();

        if(dim == 3)
            // z-dimension
            buffer[bufferIndex++]
            =
            Fluid_Velocity_->internalField()[k].z();
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        fvPatchVectorField & UPatch =
            refCast<fvPatchVectorField>
                (
                    Fluid_Velocity_->boundaryFieldRef()[patchID]
                );

        // tmp<vectorField> patchInternalFieldTmp = UPatch.patchInternalField();
        // const vectorField & patchInternalField = patchInternalFieldTmp();

        // Write the forces to the preCICE buffer
        // For every cell of the patch
        forAll(Fluid_Velocity_->boundaryFieldRef()[patchID], i)
        {
            // Copy the force into the buffer
            // x-dimension
            buffer[bufferIndex++]
            =
            UPatch[i].x();

            // y-dimension
            buffer[bufferIndex++]
            =
            UPatch[i].y();

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                =
                UPatch[i].z();

        }

    }
}

void preciceAdapter::Volume_Coupling::Fluid_Velocity::read(double * buffer, const unsigned int dim)
{
    /* TODO: Implement
    * We need two nested for-loops for each patch,
    * the outer for the locations and the inner for the dimensions.
    * See the preCICE readBlockVectorData() implementation.
    */
    FatalErrorInFunction
        << "Reading Fluid_Velocity is not supported."
        << exit(FatalError);
}

preciceAdapter::Volume_Coupling::Fluid_Velocity::~Fluid_Velocity()
{
    delete Fluid_Velocity_ ;
}
