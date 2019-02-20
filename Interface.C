#include "Interface.H"
#include "Utilities.H"
#include "faceTriangulation.H"


using namespace Foam;

preciceAdapter::Interface::Interface
(
        precice::SolverInterface & precice,
        const fvMesh& mesh,
        std::string meshName,
        std::string locationsType,
        std::vector<std::string> patchNames
        )
    :
      precice_(precice),
      meshName_(meshName),
      locationsType_(locationsType),
      patchNames_(patchNames)
{
    // Get the meshID from preCICE
    //TODO: Don't use the suffixes in case of "Centers" or "Nodes"
    if(locationsType_ == "faceTriangles" || locationsType_ == "faceCenters" || locationsType_ == "faceCentres"){

        CentermeshID_ = precice_.getMeshID(meshName_ + "-Centers");
    }

    if(locationsType_ == "faceTriangles" || locationsType_ == "faceNodes"){

        NodemeshID_ = precice_.getMeshID(meshName_ + "-Nodes");

    }


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
}

void preciceAdapter::Interface::configureMesh(const fvMesh& mesh)
{
    // The way we configure the mesh differs between meshes based on face centers
    // and meshes based on face nodes.
    // TODO: Reduce code duplication. In the meantime, take care to update
    // all the branches.
    if (locationsType_ == "faceTriangles" || locationsType_ == "faceCenters" || locationsType_ == "faceCentres")
    {
        // Count the data locations for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numCenterLocations_ +=
                    mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres().size();
        }
        DEBUG(adapterInfo("Number of face centres: " + std::to_string(numCenterLocations_)));

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        double vertices[3 * numCenterLocations_];

        // Array of the indices of the mesh vertices.
        // Each vertex has one index, but three coordinates.
        CenterIDs_ = new int[numCenterLocations_];

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


        precice_.setMeshVertices(CentermeshID_, numCenterLocations_, vertices, CenterIDs_);


    }
    if (locationsType_ == "faceNodes" || locationsType_ == "faceTriangles")
    {
        // Count the data locations for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numNodeLocations_ +=
                    mesh.boundaryMesh()[patchIDs_.at(j)].localPoints().size();
        }
        DEBUG(adapterInfo("Number of face nodes: " + std::to_string(numNodeLocations_)));

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        double vertices[3 * numNodeLocations_];

        // Array of the indices of the mesh vertices.
        // Each vertex has one index, but three coordinates.
        NodeIDs_ = new int[numNodeLocations_];

        // Initialize the index of the vertices array
        int verticesIndex = 0;

        // Get the locations of the mesh vertices (here: face nodes)
        // for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            // Get the face nodes of the current patch
            // TODO: Check if this is correct.
            // TODO: Check if this behaves correctly in parallel.
            // TODO: Check if this behaves correctly with multiple, connected patches.
            // TODO: Maybe this should be a pointVectorField?
            const pointField & faceNodes =
                    mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();

            // Assign the (x,y,z) locations to the vertices
            // TODO: Ensure consistent order when writing/reading
            for (int i = 0; i < faceNodes.size(); i++)
            {
                vertices[verticesIndex++] = faceNodes[i].x();
                vertices[verticesIndex++] = faceNodes[i].y();
                vertices[verticesIndex++] = faceNodes[i].z();
            }
        }


        // Pass the mesh vertices information to preCICE
        precice_.setMeshVertices(NodemeshID_, numNodeLocations_, vertices, NodeIDs_);


        //Only set the triangles, when it is necessary
        if (locationsType_ == "faceTriangles"){
            {

                for (uint j = 0; j < patchIDs_.size(); j++)
                {

                    //Define triangles

                    const List<face> faceField=mesh.boundaryMesh()[patchIDs_.at(j)].localFaces();

                    const Field<point> pointCoords= mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();

                    //Array to transform coords in precice IDs
                    double triCoords[faceField.size()*18];

                    unsigned int coordIndex=0;

                    forAll(faceField,facei){

                        const face& quad_face=faceField[facei];

                        faceTriangulation faceTri(pointCoords,quad_face,false);

                        for(uint triaIndex=0; triaIndex<2; triaIndex++){
                            for(uint nodeIndex=0; nodeIndex<3; nodeIndex++){
                                for(uint xyz=0; xyz<3; xyz++)
                                    triCoords[coordIndex++]=pointCoords[faceTri[triaIndex][nodeIndex]][xyz];
                            }
                        }
                    }


                    //Number of precice IDs
                    int triaVertIDs[faceField.size()*6];

                    //Get precice IDs
                    precice_.getMeshVertexIDsFromPositions(NodemeshID_,faceField.size()*6,triCoords,triaVertIDs);


                    Info<<"Number of Triangles to set "<<faceField.size()*2<<endl;

                    //Set Triangles
                    for(int faceI=0; faceI<faceField.size()*2; faceI++){
                        precice_.setMeshTriangleWithEdges(NodemeshID_,triaVertIDs[faceI*3],triaVertIDs[faceI*3+1], triaVertIDs[faceI*3+2]);
                    }
                }
            }

        }
        if (!(locationsType_ == "faceNodes" || locationsType_ == "faceTriangles"
              || locationsType_ == "faceCenters" || locationsType_ == "faceCentres") )
        {
            FatalErrorInFunction
                    << "ERROR: interface points location type "
                    << locationsType_
                    << " is invalid."
                    << exit(FatalError);
        }
    }
}

//the readers and writers distinguish, where to use
//the Centers or Nodes in case of the Triangles specification
void preciceAdapter::Interface::addCouplingDataWriter
(
        std::string dataName,
        CouplingDataUser * couplingDataWriter
        )
{
    if(locationsType_ == "faceCentres" || locationsType_ == "faceCenters" ){

        // Set the dataID (from preCICE)
        couplingDataWriter->setDataID(precice_.getDataID(dataName, CentermeshID_));
    }
    else{

        // Set the dataID (from preCICE)
        couplingDataWriter->setDataID(precice_.getDataID(dataName, NodemeshID_));
    }

    // Set the patchIDs of the patches that form the interface
    couplingDataWriter->setPatchIDs(patchIDs_);

    // Add the CouplingDataUser to the list of writers
    couplingDataWriters_.push_back(couplingDataWriter);
}


void preciceAdapter::Interface::addCouplingDataReader
(
        std::string dataName,
        preciceAdapter::CouplingDataUser * couplingDataReader
        )
{
    if(locationsType_ == "faceNodes"){

        // Set the patchIDs of the patches that form the interface
        couplingDataReader->setDataID(precice_.getDataID(dataName, NodemeshID_));
    }
    else{

        // Set the patchIDs of the patches that form the interface
        couplingDataReader->setDataID(precice_.getDataID(dataName, CentermeshID_));
    }
    // Add the CouplingDataUser to the list of readers
    couplingDataReader->setPatchIDs(patchIDs_);
    couplingDataReaders_.push_back(couplingDataReader);
}

void preciceAdapter::Interface::createBuffer()
{
    // Will the interface buffer need to store 3D vector data?
    bool needsVectorData = false;
    int dataBufferSize = 0;

    // Check all the coupling data readers
    for (uint i = 0; i < couplingDataReaders_.size(); i++)
    {
        if (couplingDataReaders_.at(i)->hasVectorData())
        {
            needsVectorData = true;
        }
    }

    // Check all the coupling data writers
    for (uint i = 0; i < couplingDataWriters_.size(); i++)
    {
        if (couplingDataWriters_.at(i)->hasVectorData())
        {
            needsVectorData = true;
        }
    }

    // Set the appropriate buffer size
    if (needsVectorData)
    {
        if(locationsType_== "faceNodes" || locationsType_ == "faceTriangles")
            dataBufferSize = 3*numNodeLocations_;
        else
            dataBufferSize = 3*numCenterLocations_;

    }
    else
    {
        if(locationsType_== "faceNodes" || locationsType_ == "faceTriangles")
            dataBufferSize = numNodeLocations_;
        else
            dataBufferSize = numCenterLocations_;
    }

    // Create the data buffer
    // An interface has only one data buffer, which is shared between several
    // CouplingDataUsers.
    // TODO: Check (write tests) if this works properly when we have multiple
    // scalar and vector coupling data users in an interface. With the current
    // preCICE implementation, it should work as, when writing scalars,
    // it should  only use the first 1/3 elements of the buffer.
    dataBuffer_ = new double[dataBufferSize]();
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
                if(locationsType_== "faceNodes"){

                    precice_.readBlockVectorData
                            (
                                couplingDataReader->dataID(),
                                numNodeLocations_,
                                NodeIDs_,
                                dataBuffer_
                                );
                }
                else if(locationsType_== "faceCentres" || locationsType_ == "faceCenters" || locationsType_ == "faceTriangles"){
                    precice_.readBlockVectorData
                            (
                                couplingDataReader->dataID(),
                                numCenterLocations_,
                                CenterIDs_,
                                dataBuffer_
                                );
                }

            }
            else
            {
                if(locationsType_== "faceNodes"){

                    precice_.readBlockScalarData
                            (
                                couplingDataReader->dataID(),
                                numNodeLocations_,
                                NodeIDs_,
                                dataBuffer_
                                );
                }
                else if(locationsType_== "faceCentres" || locationsType_ == "faceCenters"  || locationsType_ == "faceTriangles"){

                    precice_.readBlockScalarData
                            (
                                couplingDataReader->dataID(),
                                numCenterLocations_,
                                CenterIDs_,
                                dataBuffer_
                                );

                }
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

            if(locationsType_== "faceNodes" || locationsType_ == "faceTriangles"){

                precice_.writeBlockVectorData
                        (
                            couplingDataWriter->dataID(),
                            numNodeLocations_,
                            NodeIDs_,
                            dataBuffer_
                            );
            }
            else if(locationsType_== "faceCentres" || locationsType_ == "faceCenters"){

                precice_.writeBlockVectorData
                        (
                            couplingDataWriter->dataID(),
                            numCenterLocations_,
                            CenterIDs_,
                            dataBuffer_
                            );
            }
        }
        else
        {

            if(locationsType_== "faceNodes" || locationsType_ == "faceTriangles"){

                precice_.writeBlockScalarData
                        (
                            couplingDataWriter->dataID(),
                            numNodeLocations_,
                            NodeIDs_,
                            dataBuffer_
                            );
            }
            else if(locationsType_== "faceCentres" || locationsType_ == "faceCenters"){
                precice_.writeBlockScalarData
                        (
                            couplingDataWriter->dataID(),
                            numCenterLocations_,
                            CenterIDs_,
                            dataBuffer_
                            );

            }
        }
    }

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

    // Delete the IDs_
    delete [] NodeIDs_;
    delete [] CenterIDs_;

    // Delete the shared data buffer
    delete [] dataBuffer_;

}
