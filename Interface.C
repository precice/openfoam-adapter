#include "Interface.H"
#include "cellSet.H"
//#include "patchPointToCell/patchPointToCell.H"

using namespace Foam;

preciceAdapter::Interface::Interface
(
    precice::SolverInterface & precice,
    const fvMesh& mesh,
    std::string meshName,
    std::vector<std::string> patchNames,
    std::vector<std::string> cellSetNames
)
:
precice_(precice),
meshName_(meshName),
patchNames_(patchNames),
cellSetNames_(cellSetNames)
{
    // Get the meshID from preCICE
    meshID_ = precice_.getMeshID(meshName_);

    // For every patch that participates in the coupling
    for (uint j = 0; j < patchNames.size(); j++)
    {
        // Get the patchID
        int patchID = mesh.boundaryMesh().findPatchID(patchNames.at(j));

        // Throw an error if the patch was not found
        if (patchID == -1)
        {
            FatalErrorInFunction
                 << "ERROR: Patch '"
                 << patchNames.at(j)
                 << "' does not exist."
                 << exit(FatalError);
        }

        // Add the patch in the list
        patchIDs_.push_back(patchID);
    }

    // Configure the mesh (set the data locations)
    configureMesh(mesh);

    //  An interface has only one data buffer, which is shared between several
    //  CouplingDataUsers.
    //  The initial allocation assumes scalar data.
    //  If CouplingDataUsers have vector data, it is resized.
    dataBuffer_ = new double[numDataLocations_]();
}

void preciceAdapter::Interface::configureMesh(const fvMesh& mesh)
{

	// Get the cell labels of the overlapping region
	std::vector<labelList> overlapCells;
    // For every cellSet that participates in the coupling
	for (uint j = 0; j < cellSetNames_.size(); j++)
	{
		// Create a cell set
		cellSet overlapRegion(mesh, cellSetNames_[j]);

		// Throw an error if the patch was not found
//		if (overlapRegion == NULL)
//		{
//			FatalErrorInFunction
//				 << "ERROR: CellSet'"
//				 << cellSetNames.at(j)
//				 << "' does not exist."
//				 << exit(FatalError);
//		}

		// Add the cells ID's to the vector and count how many overlap cells does the interface has
		overlapCells.push_back(overlapRegion.toc());
		numDataLocations_ += overlapCells[j].size();
	}

    // Count the data locations for all the patches
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
    	// The number of cells attached with the boundary patch is added to numDataLocations.
        numDataLocations_ +=
            mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres().size();// +
            //mesh.boundaryMesh()[patchIDs_.at(j)].faceCells().size();
    }

    // Array of the mesh vertices.
    // One mesh is used for all the patches and each vertex has 3D coordinates.
    double vertices[3 * numDataLocations_];

    // Array of the indices of the mesh vertices.
    // Each vertex has one index, but three coordinates.
    vertexIDs_ = new int[numDataLocations_];

    // Initialize the index of the vertices array
    int verticesIndex = 0;

    // Get the locations of the mesh vertices (here: face centers)
    // for all the patches and the overlapping cells (cellSets)
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        // Get the face centers of the current patch
        const vectorField & faceCenters =
            mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres();

        // Assign the (x,y,z) locations to the vertices
        for (int i = 0; i < faceCenters.size(); i++)
        {
            vertices[verticesIndex++] = faceCenters[i].x();
            vertices[verticesIndex++] = faceCenters[i].y();
            vertices[verticesIndex++] = faceCenters[i].z();
        }

        // Get the cell centres of the current cellSet.
        const labelList & cells = overlapCells.at(j);

        // Get the coordinates of the cells of the current cellSet.
        for (int i=0; i < cells.size(); i++)
        {
        	vertices[verticesIndex++] = mesh.C().internalField()[cells[i]].x();
        	vertices[verticesIndex++] = mesh.C().internalField()[cells[i]].y();
        	vertices[verticesIndex++] = mesh.C().internalField()[cells[i]].z();
        }

    }

    std::cout << numDataLocations_ << std::endl;
    for (int i =0; i< (numDataLocations_); i++ )
    	std::cout << vertices[3*i] << " " << vertices[3*i+1] << " " << vertices[3*i+2] << std::endl;

    // Pass the mesh vertices information to preCICE
    precice_.setMeshVertices(meshID_, numDataLocations_, vertices, vertexIDs_);
}


void preciceAdapter::Interface::addCouplingDataWriter
(
    std::string dataName,
    CouplingDataUser * couplingDataWriter
)
{
    // Set the dataID (from preCICE)
    couplingDataWriter->setDataID(precice_.getDataID(dataName, meshID_));

    // Set the patchIDs of the patches that form the interface
    couplingDataWriter->setPatchIDs(patchIDs_);

    // Set the names of the cellSets that form the overlapping regions of the interface
    couplingDataWriter->setCellSetNames(cellSetNames_);

    // Add the CouplingDataUser to the list of writers
    couplingDataWriters_.push_back(couplingDataWriter);

    // Resize buffer for vector data.
    // TODO: This resizing is hard coded for 3 dimensional simulations.
    // I should get the number of dimensions of the simulation from somewhere, but I don't know where.
    if (couplingDataWriter->hasVectorData())
    {
    	couplingDataWriter->setBufferSize(3*numDataLocations_);
    	dataBuffer_ = (double*)realloc(dataBuffer_, (3*numDataLocations_)*sizeof(double));
    }
}


void preciceAdapter::Interface::addCouplingDataReader
(
    std::string dataName,
    preciceAdapter::CouplingDataUser * couplingDataReader
)
{
    // Set the patchIDs of the patches that form the interface
    couplingDataReader->setDataID(precice_.getDataID(dataName, meshID_));

    // Add the CouplingDataUser to the list of readers
    couplingDataReader->setPatchIDs(patchIDs_);
    couplingDataReader->setCellSetNames(cellSetNames_);
    couplingDataReaders_.push_back(couplingDataReader);

    // Resize buffer for vector data.
    // TODO: This resizing is hard coded for 3 dimensional simulations.
    // I should get the number of dimensions of the simulation from somewhere, but I don't know where.
    if (couplingDataReader->hasVectorData())
    {
    	couplingDataReader->setBufferSize(3*numDataLocations_);
    	dataBuffer_ = (double*)realloc(dataBuffer_, (3*numDataLocations_)*sizeof(double));
    }
}


void preciceAdapter::Interface::readCouplingData()
{
    // Are new data available or is the participant subcycling?
    if (precice_.isReadDataAvailable())
    {
        // Make every coupling data reader read
        for (uint i = 0; i < couplingDataReaders_.size(); i++)
        {
            // Pointer to the current reader
            preciceAdapter::CouplingDataUser *
                couplingDataReader = couplingDataReaders_.at(i);

            // Make preCICE read vector or scalar data
            // and fill the adapter's buffer
            if (couplingDataReader->hasVectorData())
            {
                precice_.readBlockVectorData
                (
                    couplingDataReader->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_
                );

                // DEBUG! WRITING OF THE DATA RECEIVED FROM LUMIS
               // std::cout << " Data received from LUMIS " << std::endl;
                //for (int i=0; i<numDataLocations_; i++)
                	//std::cout << dataBuffer_[3*i] << " " << dataBuffer_[3*i+1] << " " << dataBuffer_[3*i+2] << std::endl;

            }
            else
            {
                precice_.readBlockScalarData
                (
                    couplingDataReader->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_
                );
            }

            // Read the received data from the buffer
            couplingDataReader->read(dataBuffer_);
        }
    }
}

void preciceAdapter::Interface::writeCouplingData()
{
    // TODO: wrap around isWriteDataRequired
    // Does the participant need to write data or is it subcycling?
    // if (precice_.isWriteDataRequired(computedTimestepLength))
    // {
        // Make every coupling data writer write
        for (uint i = 0; i < couplingDataWriters_.size(); i++)
        {
            // Pointer to the current reader
            preciceAdapter::CouplingDataUser *
                couplingDataWriter = couplingDataWriters_.at(i);

            // Write the data into the adapter's buffer
            couplingDataWriter->write(dataBuffer_);


            // Make preCICE write vector or scalar data
            if (couplingDataWriter->hasVectorData())
            {
            	//std::cout << "Data sent to LUMIS " << std::endl;
            	//for (int point = 0 ; point < numDataLocations_; point++)
            		//std::cout << dataBuffer_[3*point] << " " << dataBuffer_[3* point+1] << " " << dataBuffer_[3*point+2]  << std::endl;

                precice_.writeBlockVectorData
                (
                    couplingDataWriter->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_
                );

            }
            else
            {
                precice_.writeBlockScalarData
                (
                    couplingDataWriter->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_
                );

            }
        }
    // }
}

preciceAdapter::Interface::~Interface()
{
    // Delete all the coupling data readers
    for (uint i = 0; i < couplingDataReaders_.size(); i++)
    {
        delete couplingDataReaders_.at(i);
    }
    couplingDataReaders_.clear();

    // Delete all the coupling data writers
    for (uint i = 0; i < couplingDataWriters_.size(); i++)
    {
        delete couplingDataWriters_.at(i);
    }
    couplingDataWriters_.clear();

    // Delete the vertexIDs_
    delete [] vertexIDs_;

    // Delete the shared data buffer
    delete [] dataBuffer_;

}
