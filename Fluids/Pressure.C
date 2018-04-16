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


	// Get the LB pressure at the centre of the overlapping region.
	// I will get the LB pressure for the first patch only.
	int patchID = patchIDs_.at(0);
	// Ignore the boundary patch.
	int numCells = mesh_->boundaryMesh()[patchID].faceCells().size();
	int centralCell = numCells + numCells/2;

	// Caclulate pressure difference in the central cell
	const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
	double pDiff = buffer[centralCell] - P_->ref()[cells[numCells/2]];

	std::cout << "Pressure difference: " << pDiff << std::endl;

	// Set the overlapping region pressure values
	for (uint j = 0; j < patchIDs_.size(); j++)
	{
		int patchID = patchIDs_.at(j);

		 // For every cell associated to the patch
		const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
		for( int i=0; i < cells.size(); i++)
		{
			// Correct the pressure value
			P_->ref()[cells[i]] += pDiff;
		}
	}

}
