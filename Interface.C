#include "Interface.H"

using namespace Foam;

preciceAdapter::Interface::Interface
(
    precice::SolverInterface & precice,
    const fvMesh& mesh,
    std::string meshName,
    std::vector<std::string> patchNames
)
:
precice_(precice),
meshName_(meshName),
patchNames_(patchNames)
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
    // TODO: Implement the resizing for vector data (used in mechanical FSI)
    dataBuffer_ = new double[numDataLocations_]();
}

void preciceAdapter::Interface::configureMesh(const fvMesh& mesh)
{
    // Count the data locations for all the patches
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        numDataLocations_ +=
            mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres().size();
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
    // for all the patches
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
    }

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

    // Add the CouplingDataUser to the list of writers
    couplingDataWriters_.push_back(couplingDataWriter);

    // TODO: Resize buffer for vector data (if not already resized)
    if (couplingDataWriter->hasVectorData())
    {}
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
    couplingDataReaders_.push_back(couplingDataReader);

    // TODO: Resize buffer for vector data (if not already resized)
    if (couplingDataReader->hasVectorData())
    {}
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
