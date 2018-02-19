#include "Pressure.H"

using namespace Foam;

preciceAdapter::Fluids::Pressure::Pressure
(
    const Foam::fvMesh* mesh,
    const std::string nameT
)
:
P_(
    const_cast<volScalarField*>
    (
        &mesh->lookupObject<volScalarField>(nameT)
    )
)
{
    dataType_ = scalar;
    mesh_ = mesh;
}

void preciceAdapter::Fluids::Pressure::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(P_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++]
            =
            P_->boundaryFieldRef()[patchID][i];
        }

        // For every cell associated to the patch
        const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
        for( int i=0; i < cells.size(); i++)
        {
        	// Copy the pressure into the buffer
        	buffer[bufferIndex++]
        	=
        	P_->internalField()[cells[i]];
        }
    }
}

void preciceAdapter::Fluids::Pressure::read(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(P_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            P_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }

        // I only need to read the boundary patch, which is the first part of the buffer array.
        // TODO: Maybe also read in the values of the cells associated with the boundaries?
        // Skip the cells associated with the patch (their information is not needed)
        const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
        bufferIndex += cells.size()+1;
    }


}
