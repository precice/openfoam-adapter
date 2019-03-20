#include "SinkTemperature.H"

using namespace Foam;

preciceAdapter::CHT::SinkTemperature::SinkTemperature
(
    const Foam::fvMesh& mesh,
    const std::string nameT
)
:
T_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameT)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::CHT::SinkTemperature::write(double * buffer, bool provideMeshConnectivity)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // Get the boundary field of Temperature on the patch
        fvPatchScalarField & TPatch =
            refCast<fvPatchScalarField>
            (
                T_->boundaryFieldRef()[patchID]
            );

        // Get the internal field next to the patch // TODO: Simplify?
        tmp<scalarField> patchInternalFieldTmp = TPatch.patchInternalField();
        const scalarField & patchInternalField = patchInternalFieldTmp();

        // For every cell of the patch
        forAll(TPatch, i)
        {
            // Copy the internal field (sink) temperature into the buffer
            buffer[bufferIndex++]
            =
            patchInternalField[i];
        }

        // Clear the temporary internal field object
        patchInternalFieldTmp.clear();
    }
}

void preciceAdapter::CHT::SinkTemperature::read(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the boundary field of the temperature on the patch
        mixedFvPatchScalarField & TPatch =
            refCast<mixedFvPatchScalarField>
            (
                T_->boundaryFieldRef()[patchID]
            );

        // Get a reference to the reference value on the patch
        scalarField & Tref = TPatch.refValue();

        // For every cell of the patch
        forAll(TPatch, i)
        {
            // Set the reference value as the buffer value
            Tref[i]
            =
            buffer[bufferIndex++];
        }
    }
}
