#include "HU_SWE.H"

using namespace Foam;

preciceAdapter::FF::hu_SWE::hu_SWE
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

void preciceAdapter::FF::hu_SWE::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{ //TODO
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch 
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer only in the x-dimension
            buffer[bufferIndex++]
            =
            U_->boundaryFieldRef()[patchID][i].x();

            if(dim == 3)
                // z-dimension
                buffer[bufferIndex++]
                =
                U_->boundaryFieldRef()[patchID][i].z();
        }
    }
}

void preciceAdapter::FF::hu_SWE::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer only in the x-dimension
            U_->boundaryFieldRef()[patchID][i].x()
            =
            buffer[bufferIndex++];


            if(dim == 3)
                // z-dimension
                U_->boundaryFieldRef()[patchID][i].z()
                = 0.0;
        }
    }
}
