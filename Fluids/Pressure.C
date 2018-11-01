#include "Pressure.H"
#include "cellSet.H"
//#include "readGradientFvPatchField.H"

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

	int bufferIndex = 0;

	// Set the overlapping region pressure values
	for (uint j = 0; j < patchIDs_.size(); j++)
	{
		int patchID = patchIDs_.at(j);

		// Check that the boundary patch is of the correct type.
		if ( P_->boundaryFieldRef()[patchID].type()=="fixedGradient")
		{
			Field<double> grad = P_->boundaryFieldRef()[patchID];
			// For every cell of the patch
			forAll(grad, i)
			{
				// Copy the pressure gradient into grad
				grad[i] = buffer[bufferIndex++];
			}
			fixedGradientFvPatchField<double>* pBoundary = dynamic_cast<fixedGradientFvPatchField<double>*>(&P_->boundaryFieldRef()[patchID]);
			pBoundary->gradient() = grad;

			//std::cout << "Pressure gradient set from LUMIS data" << std::endl;
			//for (int i =0; i< (grad.size()); i++ )
			//	std::cout << ue->gradient()[i]  << std::endl;
		}
		else
			std::cout << " Current patch is of type " << P_->boundaryFieldRef()[patchID].type() << " and can't accept pressure gradient data." << std::endl;

	}

}
