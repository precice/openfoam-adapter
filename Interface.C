#include "Interface.H"
#include "Utilities.H"
#include "faceTriangulation.H"


using namespace Foam;

preciceAdapter::Interface::Interface(
    precice::SolverInterface& precice,
    const fvMesh& mesh,
    std::string meshName,
    std::string locationsType,
    std::vector<std::string> patchNames,
    bool meshConnectivity)
: precice_(precice),
  meshName_(meshName),
  locationsType_(locationsType),
  patchNames_(patchNames),
  meshConnectivity_(meshConnectivity)
{
    // Get the meshID from preCICE
    meshID_ = precice_.getMeshID(meshName_);

    dim_ = precice_.getDimensions();

    if (dim_ == 2 && meshConnectivity_ == true)
    {
        DEBUG(adapterInfo("meshConnectivity is currently only supported for 3D cases. \n"
                          "You might set up a 3D case and restrict the 3rd dimension by z-dead = true. \n"
                          "Have a look in the adapter documentation for detailed information.",
                          "warning"));
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

    if (locationsType_ == "faceCenters" || locationsType_ == "faceCentres")
    {
        // Count the data locations for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numDataLocations_ +=
                mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres().size();
        }
        DEBUG(adapterInfo("Number of face centres: " + std::to_string(numDataLocations_)));

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        double vertices[dim_ * numDataLocations_];

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
            const vectorField faceCenters =
                mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres();

            // Assign the (x,y,z) locations to the vertices
            for (int i = 0; i < faceCenters.size(); i++)
                for (unsigned int d = 0; d < dim_; ++d)
                    vertices[verticesIndex++] = faceCenters[i][d];

            // Check if we are in the right layer in case of preCICE dimension 2
            // If there is at least one node with a different z-coordinate, then the (2D) geometry is not on the xy-plane, as required.
            if (dim_ == 2)
            {
                const pointField faceNodes =
                    mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();
                // Allocate memory for z-coordinates
                std::array<double, 2> z_location({0, 0});
                constexpr unsigned int z_axis = 2;

                // Find out about the existing planes
                // Store z-coordinate of the first layer
                z_location[0] = faceNodes[0][z_axis];
                // Go through the remaining points until we find the second z-coordinate
                // and store it (there are only two allowed in case we are in the xy-layer)
                for (int i = 0; i < faceNodes.size(); i++)
                {
                    if (z_location[0] == faceNodes[i][z_axis])
                    {
                        continue;
                    }
                    else
                    {
                        z_location[1] = faceNodes[i][z_axis];
                        break;
                    }
                }

                // Check if the z-coordinates of all nodes match the z-coordinates we have collected above
                for (int i = 0; i < faceNodes.size(); i++)
                {
                    if (z_location[0] == faceNodes[i][z_axis] || z_location[1] == faceNodes[i][z_axis])
                    {
                        continue;
                    }
                    else
                    {
                        FatalErrorInFunction
                            << "It seems like you are using preCICE in 2D and your geometry is not located int the xy-plane. "
                               "The OpenFOAM adapter implementation supports preCICE 2D cases only with the z-axis as out-of-plane direction."
                               "Please rotate your geometry so that the geometry is located in the xy-plane."
                            << exit(FatalError);
                    }
                }
            }
        }

        // Pass the mesh vertices information to preCICE
        precice_.setMeshVertices(meshID_, numDataLocations_, vertices, vertexIDs_);
    }
    else if (locationsType_ == "faceNodes")
    {
        // Count the data locations for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numDataLocations_ +=
                mesh.boundaryMesh()[patchIDs_.at(j)].localPoints().size();
        }
        DEBUG(adapterInfo("Number of face nodes: " + std::to_string(numDataLocations_)));

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        double vertices[dim_ * numDataLocations_];

        // Array of the indices of the mesh vertices.
        // Each vertex has one index, but three coordinates.
        vertexIDs_ = new int[numDataLocations_];

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
            const pointField faceNodes =
                mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();

            // Assign the (x,y,z) locations to the vertices
            // TODO: Ensure consistent order when writing/reading
            for (int i = 0; i < faceNodes.size(); i++)
                for (unsigned int d = 0; d < dim_; ++d)
                    vertices[verticesIndex++] = faceNodes[i][d];
        }

        // Pass the mesh vertices information to preCICE
        precice_.setMeshVertices(meshID_, numDataLocations_, vertices, vertexIDs_);

        // meshConnectivity for prototype neglected
        // Only set the triangles, if necessary
        if (meshConnectivity_)
        {
            for (uint j = 0; j < patchIDs_.size(); j++)
            {
                // Define triangles
                // This is done in the following way:
                // We get a list of faces, which belong to this patch, and triangulate each face
                // using the faceTriangulation object.
                // Afterwards, we store the coordinates of the triangulated faces in order to use
                // the preCICE function "getMeshVertexIDsFromPositions". This function returns
                // for each point the respective preCICE related ID.
                // These IDs are consequently used for the preCICE function "setMeshTriangleWithEdges",
                // which defines edges and triangles on the interface. This connectivity information
                // allows preCICE to provide a nearest-projection mapping.
                // Since data is now related to nodes, volume fields (e.g. heat flux) needs to be
                // interpolated in the data classes (e.g. CHT)

                // Define constants
                const int triaPerQuad = 2;
                const int nodesPerTria = 3;
                const int componentsPerNode = 3;

                // Get the list of faces and coordinates at the interface patch
                const List<face> faceField = mesh.boundaryMesh()[patchIDs_.at(j)].localFaces();
                const Field<point> pointCoords = mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();

                // Array to store coordiantes in preCICE format
                double triCoords[faceField.size() * triaPerQuad * nodesPerTria * componentsPerNode];

                unsigned int coordIndex = 0;

                // Iterate over faces
                forAll(faceField, facei)
                {
                    const face& faceQuad = faceField[facei];

                    faceTriangulation faceTri(pointCoords, faceQuad, false);

                    for (uint triIndex = 0; triIndex < triaPerQuad; triIndex++)
                    {
                        for (uint nodeIndex = 0; nodeIndex < nodesPerTria; nodeIndex++)
                        {
                            for (uint xyz = 0; xyz < componentsPerNode; xyz++)
                                triCoords[coordIndex++] = pointCoords[faceTri[triIndex][nodeIndex]][xyz];
                        }
                    }
                }

                // Array to store the IDs we get from preCICE
                int triVertIDs[faceField.size() * (triaPerQuad * nodesPerTria)];

                // Get preCICE IDs
                precice_.getMeshVertexIDsFromPositions(meshID_, faceField.size() * (triaPerQuad * nodesPerTria), triCoords, triVertIDs);

                DEBUG(adapterInfo("Number of triangles: " + std::to_string(faceField.size() * triaPerQuad)));

                // Set Triangles
                for (int facei = 0; facei < faceField.size() * triaPerQuad; facei++)
                {
                    precice_.setMeshTriangleWithEdges(meshID_, triVertIDs[facei * nodesPerTria], triVertIDs[facei * nodesPerTria + 1], triVertIDs[facei * nodesPerTria + 2]);
                }
            }
        }
    }
    else if (locationsType_ == "volume")
    {
        // module for volume coupling is based on the module for coupling faceCenters
        // as the module was tested and developed on a previous development where faceNodes
        // did not work properly
        //
        // the module for volume coupling considers the mesh points in the volume and
        // on the boundary patches in order to take the boundary conditions into account

        // get the number (volume centered) mesh points in the volume
        numDataLocations_ = mesh.C().size();

        // Count the data locations for all the patches
        // and add the those to the previously determined number of mesh points in the volume
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numDataLocations_ +=
                mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres().size();
        }
        DEBUG(adapterInfo("Number of coupling volumes: " + std::to_string(numDataLocations_)));

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        double vertices[dim_ * numDataLocations_];

        // Array of the indices of the mesh vertices.
        // Each vertex has one index, but three coordinates.
        vertexIDs_ = new int[numDataLocations_];

        // Initialize the index of the vertices array
        int verticesIndex = 0;

        // Get the locations of the volume centered mesh vertices
        const vectorField& CellCenters = mesh.C();

        for (int i = 0; i < CellCenters.size(); i++)
        {
            vertices[verticesIndex++] = CellCenters[i].x();
            vertices[verticesIndex++] = CellCenters[i].y();
            if (dim_ == 3)
            {
                vertices[verticesIndex++] = CellCenters[i].z();
            }
        }

        // Get the locations of the mesh vertices (here: face centers)
        // for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            // Get the face centers of the current patch
            const vectorField faceCenters =
                mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres();

            // Assign the (x,y,z) locations to the vertices
            for (int i = 0; i < faceCenters.size(); i++)
            {
                vertices[verticesIndex++] = faceCenters[i].x();
                vertices[verticesIndex++] = faceCenters[i].y();
                if (dim_ == 3)
                {
                    vertices[verticesIndex++] = faceCenters[i].z();
                }
            }
        }

        // Pass the mesh vertices information to preCICE
        precice_.setMeshVertices(meshID_, numDataLocations_, vertices, vertexIDs_);
    }
    else
    {
        FatalErrorInFunction
            << "ERROR: interface points location type "
            << locationsType_
            << " is invalid."
            << exit(FatalError);
    }
}


void preciceAdapter::Interface::addCouplingDataWriter(
    std::string dataName,
    CouplingDataUser* couplingDataWriter)
{
    // Set the dataID (from preCICE)
    couplingDataWriter->setDataID(precice_.getDataID(dataName, meshID_));

    // Set the patchIDs of the patches that form the interface
    couplingDataWriter->setPatchIDs(patchIDs_);

    // Set the location type in the CouplingDataUser class
    couplingDataWriter->setLocationsType(locationsType_);

    // Initilaize class specific data
    couplingDataWriter->initialize();

    // Add the CouplingDataUser to the list of writers
    couplingDataWriters_.push_back(couplingDataWriter);
}


void preciceAdapter::Interface::addCouplingDataReader(
    std::string dataName,
    preciceAdapter::CouplingDataUser* couplingDataReader)
{
    // Set the patchIDs of the patches that form the interface
    couplingDataReader->setDataID(precice_.getDataID(dataName, meshID_));

    // Add the CouplingDataUser to the list of readers
    couplingDataReader->setPatchIDs(patchIDs_);

    // Set the location type in the CouplingDataUser class
    couplingDataReader->setLocationsType(locationsType_);

    // Initilaize class specific data
    couplingDataReader->initialize();

    // Add the CouplingDataUser to the list of readers
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
        dataBufferSize = dim_ * numDataLocations_;
    }
    else
    {
        dataBufferSize = numDataLocations_;
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
            preciceAdapter::CouplingDataUser*
                couplingDataReader = couplingDataReaders_.at(i);

            // Make preCICE read vector or scalar data
            // and fill the adapter's buffer
            if (couplingDataReader->hasVectorData())
            {
                precice_.readBlockVectorData(
                    couplingDataReader->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_);
            }
            else
            {
                precice_.readBlockScalarData(
                    couplingDataReader->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_);
            }

            // Read the received data from the buffer
            couplingDataReader->read(dataBuffer_, dim_);
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
        preciceAdapter::CouplingDataUser*
            couplingDataWriter = couplingDataWriters_.at(i);

        // Write the data into the adapter's buffer
        couplingDataWriter->write(dataBuffer_, meshConnectivity_, dim_);

        // Make preCICE write vector or scalar data
        if (couplingDataWriter->hasVectorData())
        {
            precice_.writeBlockVectorData(
                couplingDataWriter->dataID(),
                numDataLocations_,
                vertexIDs_,
                dataBuffer_);
        }
        else
        {
            precice_.writeBlockScalarData(
                couplingDataWriter->dataID(),
                numDataLocations_,
                vertexIDs_,
                dataBuffer_);
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
    delete[] vertexIDs_;

    // Delete the shared data buffer
    delete[] dataBuffer_;
}
