#include "HV_SWE.H"

using namespace Foam;

preciceAdapter::FF::hv_SWE::hv_SWE
(
    const Foam::fvMesh& mesh,
    const std::string nameU
)
:
U_(
    const_cast<volVectorField*>
    (
        &mesh.lookupObject<volVectorField>(nameU)
    )
)
{
    dataType_ = vector;
}

void preciceAdapter::FF::hv_SWE::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch TODO
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            // // x-dimension
            // buffer[bufferIndex++]
            // =
            // U_->boundaryFieldRef()[patchID][i].x();

            // y-dimension
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i].y();

            // if(dim == 3)
            //     // z-dimension
            //     buffer[bufferIndex++]
            //     =
            //     U_->boundaryFieldRef()[patchID][i].z();
        }
    }
}

void preciceAdapter::FF::hv_SWE::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            U_->boundaryFieldRef()[patchID][i].x()
            =
            buffer[bufferIndex++];

            // // y-dimension
            // U_->boundaryFieldRef()[patchID][i].y()
            // =
            // buffer[bufferIndex++];

            if(dim == 3)
                // z-dimension
                U_->boundaryFieldRef()[patchID][i].z()
                =
                buffer[bufferIndex++];
        }
    }
}
