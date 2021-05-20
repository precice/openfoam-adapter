#include "Your_Volume_Field.H"

using namespace Foam;

preciceAdapter::Momentum::Your_Volume_Field::Your_Volume_Field
(
        const Foam::fvMesh& mesh,
        const std::string nameYour_Volume_Field
)
:
Your_Volume_Field_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameYour_Volume_Field)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::Momentum::Your_Volume_Field::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    FatalErrorInFunction
        << "Writing porosity is not supported."
        << exit(FatalError);
}

void preciceAdapter::Momentum::Your_Volume_Field::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // for every internal cell
    forAll(Your_Volume_Field_->ref(), k)
    {
        Your_Volume_Field_->ref()[k]
        =
        buffer[bufferIndex++] ;
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the boundary field of Porosity on the patch
        fvPatchScalarField & PPatch =
            refCast<fvPatchScalarField>
            (
                Your_Volume_Field_->boundaryFieldRef()[patchID]
            );

        //tmp<scalarField> patchInternalFieldTmp = PPatch.patchInternalField();
        //const scalarField & patchInternalField = patchInternalFieldTmp();


        // For every cell of the patch
        forAll(Your_Volume_Field_->boundaryFieldRef()[patchID], i)
        {
            // Copy the internal field porosity into the buffer
            PPatch[i]
            =
            buffer[bufferIndex++];

        }
    }



}
