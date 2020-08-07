#include "Height_SWE.H"

using namespace Foam;

preciceAdapter::FF::height_SWE::height_SWE
(
    const Foam::fvMesh& mesh,
    const std::string nameA_
)
:
alpha_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameA_)
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::FF::height_SWE::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch TODO
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
            // Copy the HU into the buffer
            buffer[bufferIndex++]
            =
            alpha_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::height_SWE::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);



        // For every cell of the patch
        //TODO
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {

            // Set the HU as the buffer value
            alpha_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }
    }
}
