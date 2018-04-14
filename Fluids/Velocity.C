#include "Velocity.H"

using namespace Foam;

preciceAdapter::Fluids::Velocity::Velocity
(
    const Foam::fvMesh* mesh,
    const std::string nameT,
    const double vDot
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
    vDot_ = vDot;
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

double preciceAdapter::Fluids::Velocity::massCorrection(double * buffer, int patchID)
{
	// Calculate volumetric flow rate from the LUMIS data.
	double flowRate = 0;

	const polyPatch& cPatch = mesh_->boundaryMesh()[patchID];

	// For every cell of the patch
	//double area = 0;
	forAll(cPatch, i)
	{
		flowRate += buffer[3*i]*mesh_->magSf().boundaryField()[patchID][i];

		//area += mesh_->magSf().boundaryField()[patchID][i];
	}
	//std::cout << " Flow rate: " << flowRate  << " Area: " << area << std::endl;
	std::cout << " Flow rate: " << flowRate  << " Target flow rate: " << vDot_ << std::endl;

	return vDot_/flowRate;
}

void preciceAdapter::Fluids::Velocity::read(double * buffer)
{


    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Mass correction
        //TODO: Not sure this will work if you have more than one patch. Not sure if the way I'm calculating the correction will be accurate anymore.
		float mcorrection = 1.0;
		if(vDot_ > 0)
			mcorrection = massCorrection(buffer, patchID);

		std::cout << " Mass correction: " << mcorrection << std::endl;

		int bufferIndex = 0;

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            U_->boundaryFieldRef()[patchID][i].x() = mcorrection * buffer[bufferIndex++];
            U_->boundaryFieldRef()[patchID][i].y() = buffer[bufferIndex++];
            U_->boundaryFieldRef()[patchID][i].z() = buffer[bufferIndex++];

            // Check that the mass flow has been corrected?
        }

        // For every cell associated to the patch.
        // TODO: I'm not applying the mass correction to this cells (anyway, their values will be overwritten by OF on the next time step)
        // but not applying mass correction might affect the results.
        const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
		for( int i=0; i < cells.size(); i++)
		{
			// Set the velocity to the buffer value
			U_->ref()[cells[i]].x() = buffer[bufferIndex++];
			U_->ref()[cells[i]].y() = buffer[bufferIndex++];
			U_->ref()[cells[i]].z() = buffer[bufferIndex++];
		}

        // Skip the cells associated with the patch (their information is not needed)
        //const labelList & cells = mesh_->boundaryMesh()[patchID].faceCells();
        //bufferIndex += cells.size()+1;
    }
}
