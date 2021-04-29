#include "SinkTemperature.H"
#include "primitivePatchInterpolation.H"

using namespace Foam;

preciceAdapter::CHT::SinkTemperature::SinkTemperature(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}

void preciceAdapter::CHT::SinkTemperature::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the boundary field of Temperature on the patch
        const fvPatchScalarField& TPatch(
            refCast<const fvPatchScalarField>(
                T_->boundaryField()[patchID]));

        // Get the internal field next to the patch // TODO: Simplify?
        tmp<scalarField> patchInternalFieldTmp = TPatch.patchInternalField();
        const scalarField& patchInternalField = patchInternalFieldTmp();

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if (meshConnectivity)
        {
            //Create an Interpolation object at the boundary Field
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            scalarField patchInternalPointField;

            //Interpolate from centers to nodes
            patchInternalPointField = patchInterpolator.faceToPointInterpolate(patchInternalField);

            // For every point on the patch
            forAll(patchInternalPointField, i)
            {
                // Copy the temperature into the buffer
                buffer[bufferIndex++] =
                    patchInternalPointField[i];
            }
        }
        else
        {
            // For every cell of the patch
            forAll(TPatch, i)
            {
                // Copy the internal field (sink) temperature into the buffer
                buffer[bufferIndex++] =
                    patchInternalField[i];
            }
        }

        // Clear the temporary internal field object
        patchInternalFieldTmp.clear();
    }
}

void preciceAdapter::CHT::SinkTemperature::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the boundary field of the temperature on the patch
        mixedFvPatchScalarField& TPatch(
            refCast<mixedFvPatchScalarField>(
                T_->boundaryFieldRef()[patchID]));

        // Get a reference to the reference value on the patch
        scalarField& Tref = TPatch.refValue();

        // For every cell of the patch
        forAll(TPatch, i)
        {
            // Set the reference value as the buffer value
            Tref[i] =
                buffer[bufferIndex++];
        }
    }
}
