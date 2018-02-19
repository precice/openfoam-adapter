#include "Velocity.H"

using namespace Foam;

preciceAdapter::Fluids::Velocity::Velocity
(
    const Foam::fvMesh* mesh,
    const std::string nameT
)
:
U_(
    const_cast<volVectorField*>
    (
        &mesh->lookupObject<volVectorField>(nameT)
    )
)
{
    dataType_ = vector;
    mesh_ = mesh;
}

void preciceAdapter::Fluids::Velocity::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the three components of the velocity into the buffer
            buffer[bufferIndex++] = U_->boundaryFieldRef()[patchID][i].x();
            buffer[bufferIndex++] = U_->boundaryFieldRef()[patchID][i].y();
            buffer[bufferIndex++] = U_->boundaryFieldRef()[patchID][i].z();
        }

        // For every cell associated to the patch
		const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
		for( int i=0; i < cells.size(); i++)
		{
			// Copy the pressure into the buffer
			buffer[bufferIndex++] = U_->internalField()[cells[i]].x();
			buffer[bufferIndex++] = U_->internalField()[cells[i]].y();
			buffer[bufferIndex++] = U_->internalField()[cells[i]].z();
		}
    }

}

void preciceAdapter::Fluids::Velocity::read(double * buffer)
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
            U_->boundaryFieldRef()[patchID][i].x() = buffer[bufferIndex++];
            U_->boundaryFieldRef()[patchID][i].y() = buffer[bufferIndex++];
            U_->boundaryFieldRef()[patchID][i].z() = buffer[bufferIndex++];
        }

        // I only need to read the boundary patch, which is the first part of the buffer array.
        // TODO: Maybe also read in the values of the cells associated with the boundaries?
        // Skip the cells associated with the patch (their information is not needed)
        const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
        bufferIndex += cells.size()+1;
    }
}
