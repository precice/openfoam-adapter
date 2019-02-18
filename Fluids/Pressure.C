#include "Pressure.H"
#include "cellSet.H"
//#include "readGradientFvPatchField.H"
#include "fixedGradientFvPatchField.H"

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

 	std::cout << "Reading pressure from LUMIS: " << std::endl;

	// Set the overlapping region pressure values
	for (uint j = 0; j < patchIDs_.size(); j++)
	{
		int patchID = patchIDs_.at(j);

		P_->boundaryFieldRef()[patchID].updateCoeffs();
		Field<double> grad = P_->boundaryFieldRef()[patchID];
        // For every cell of the patch
    //forAll(grad, i)
    for( int i=0; i < grad.size(); i++)
    {
      // Copy the pressure gradient into grad
         grad[i] = buffer[bufferIndex++];            
    }
        
    fixedGradientFvPatchField<double>* ue = dynamic_cast<fixedGradientFvPatchField<double>*>(&P_->boundaryFieldRef()[patchID]);
    if (ue == nullptr)
    {
      std::cout << "Dynamic cast in read method for pressure failed. Please make sure that the pressure BC in the interface is set to fixedGradient." << std::endl; 
    }
    //std::cout << "Gradient field extracted: " << std::endl;
   
    ue->gradient() = grad; 
    std::cout << "gradient field set: " << std::endl;

	}

}
