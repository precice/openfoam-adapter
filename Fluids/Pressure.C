#include "Pressure.H"
#include "cellSet.H"

using namespace Foam;

preciceAdapter::Fluids::Pressure::Pressure
(
    const Foam::fvMesh& mesh,
    const std::string nameT
)
:
P_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameT)
    )
),mesh_(mesh)
{
    dataType_ = scalar;
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

        // For every cellSet associated to the patch
        // TODO: Do I have to create the cellSet each time? Don't they have indices or something like patches do?
		// Maybe I can store pointers to the cellSets?
		cellSet overlapRegion(mesh_, cellSetNames_[j]);
		const labelList & cells = overlapRegion.toc();
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
	cellSet overlapRegion(mesh_, cellSetNames_.at(0));

	// Get the cell at the middle of the cellSet
	const labelList & cells = overlapRegion.toc();
	int numCells = cells.size();
	int centralCell = P_->boundaryFieldRef()[patchID].size() + numCells/2;

	std::cout << "Cells in patch: " << P_->boundaryFieldRef()[patchID].size() << std::endl;

	// Caclulate pressure difference in the central cell
	double pDiff = buffer[centralCell] - P_->ref()[cells[numCells/2]];

	std::cout << "Pressure difference: " << pDiff << std::endl;

	// Set the overlapping region pressure values
	for (uint j = 0; j < patchIDs_.size(); j++)
	{
		int patchID = patchIDs_.at(j);

		 // For every cell associated to the patch
		const labelList & cells = mesh_.boundaryMesh()[patchID].faceCells();
		for( int i=0; i < cells.size(); i++)
		{
			// Correct the pressure value
			P_->ref()[cells[i]] += pDiff;
		}
	}

}
