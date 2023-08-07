#include "Interface.H"
#include "Utilities.H"
#include "faceTriangulation.H"


using namespace Foam;

preciceAdapter::Interface::Interface(
    precice::Participant& precice,
    const fvMesh& mesh,
    std::string meshName,
    std::string locationsType,
    std::vector<std::string> patchNames,
    bool meshConnectivity,
    bool restartFromDeformed,
    const std::string& namePointDisplacement,
    const std::string& nameCellDisplacement)
: precice_(precice),
  meshName_(meshName),
  patchNames_(patchNames),
  meshConnectivity_(meshConnectivity),
  restartFromDeformed_(restartFromDeformed)
{
    dim_ = precice_.getMeshDimensions(meshName);

    if (dim_ == 2 && meshConnectivity_ == true)
    {
        DEBUG(adapterInfo("meshConnectivity is currently only supported for 3D cases. \n"
                          "You might set up a 3D case and restrict the 3rd dimension by z-dead = true. \n"
                          "Have a look in the adapter documentation for detailed information.",
                          "warning"));
    }

    if (locationsType == "faceCenters" || locationsType == "faceCentres")
    {
        locationType_ = LocationType::faceCenters;
    }
    else if (locationsType == "faceNodes")
    {
        locationType_ = LocationType::faceNodes;
    }
    else
    {
        adapterInfo("Interface points location type \""
                    "locations = "
                        + locationsType + "\" is invalid.",
                    "error-deferred");
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
    configureMesh(mesh, namePointDisplacement, nameCellDisplacement);
}

void preciceAdapter::Interface::configureMesh(const fvMesh& mesh, const std::string& namePointDisplacement, const std::string& nameCellDisplacement)
{
    // The way we configure the mesh differs between meshes based on face centers
    // and meshes based on face nodes.
    // TODO: Reduce code duplication. In the meantime, take care to update
    // all the branches.

    if (locationType_ == LocationType::faceCenters)
    {
        // Count the data locations for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numDataLocations_ +=
                mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres().size();
        }
        DEBUG(adapterInfo("Number of face centres: " + std::to_string(numDataLocations_)));

        // In case we want to perform the reset later on, look-up the corresponding data field name
        Foam::volVectorField const* cellDisplacement = nullptr;
        if (mesh.foundObject<volVectorField>(nameCellDisplacement))
            cellDisplacement =
                &mesh.lookupObject<volVectorField>(nameCellDisplacement);

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        std::vector<double> vertices(dim_ * numDataLocations_);

        // Array of the indices of the mesh vertices.
        // Each vertex has one index, but three coordinates.
        vertexIDs_.resize(numDataLocations_);

        // Initialize the index of the vertices array
        int verticesIndex = 0;

        // Get the locations of the mesh vertices (here: face centers)
        // for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            // Get the face centers of the current patch
            vectorField faceCenters =
                mesh.boundaryMesh()[patchIDs_.at(j)].faceCentres();

            // Move the interface according to the current values of the cellDisplacement field,
            // to account for any displacements accumulated before restarting the simulation.
            // This is information that OpenFOAM reads from its result/restart files.
            // If the simulation is not restarted, the displacement should be zero and this line should have no effect.
            if (cellDisplacement != nullptr && !restartFromDeformed_)
                faceCenters -= cellDisplacement->boundaryField()[patchIDs_.at(j)];

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
                const auto faceNodesSize = faceNodes.size();
                //Allocate memory for z-coordinates
                std::array<double, 2> z_location({0, 0});
                constexpr unsigned int z_axis = 2;

                // Find out about the existing planes
                // Store z-coordinate of the first layer
                if (faceNodesSize > 0)
                {
                    z_location[0] = faceNodes[0][z_axis];
                }
                // Go through the remaining points until we find the second z-coordinate
                // and store it (there are only two allowed in case we are in the xy-layer)
                for (int i = 0; i < faceNodesSize; i++)
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
                for (int i = 0; i < faceNodesSize; i++)
                {
                    if (z_location[0] == faceNodes[i][z_axis] || z_location[1] == faceNodes[i][z_axis])
                    {
                        continue;
                    }
                    else
                    {
                        adapterInfo("It seems like you are using preCICE in 2D and your geometry is not located int the xy-plane. "
                                    "The OpenFOAM adapter implementation supports preCICE 2D cases only with the z-axis as out-of-plane direction."
                                    "Please rotate your geometry so that the geometry is located in the xy-plane."
                                    "If you are running a 2D axisymmetric case just ignore this.",
                                    "warning");
                    }
                }
            }
        }

        // Pass the mesh vertices information to preCICE
        precice_.setMeshVertices(meshName_, vertices, vertexIDs_);
    }
    else if (locationType_ == LocationType::faceNodes)
    {
        // Count the data locations for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            numDataLocations_ +=
                mesh.boundaryMesh()[patchIDs_.at(j)].localPoints().size();
        }
        DEBUG(adapterInfo("Number of face nodes: " + std::to_string(numDataLocations_)));

        // In case we want to perform the reset later on, look-up the corresponding data field name
        Foam::pointVectorField const* pointDisplacement = nullptr;
        if (mesh.foundObject<pointVectorField>(namePointDisplacement))
            pointDisplacement =
                &mesh.lookupObject<pointVectorField>(namePointDisplacement);

        // Array of the mesh vertices.
        // One mesh is used for all the patches and each vertex has 3D coordinates.
        std::vector<double> vertices(dim_ * numDataLocations_);

        // Array of the indices of the mesh vertices.
        // Each vertex has one index, but three coordinates.
        vertexIDs_.resize(numDataLocations_);

        // Initialize the index of the vertices array
        int verticesIndex = 0;

        // Map between OpenFOAM vertices and preCICE vertex IDs
        std::map<std::tuple<double, double, double>, int> verticesMap;

        // Get the locations of the mesh vertices (here: face nodes)
        // for all the patches
        for (uint j = 0; j < patchIDs_.size(); j++)
        {
            // Get the face nodes of the current patch
            // TODO: Check if this is correct.
            // TODO: Check if this behaves correctly in parallel.
            // TODO: Check if this behaves correctly with multiple, connected patches.
            // TODO: Maybe this should be a pointVectorField?
            pointField faceNodes =
                mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();

            // Similar to the cell displacement above:
            // Move the interface according to the current values of the cellDisplacement field,
            // to account for any displacements accumulated before restarting the simulation.
            // This is information that OpenFOAM reads from its result/restart files.
            // If the simulation is not restarted, the displacement should be zero and this line should have no effect.
            if (pointDisplacement != nullptr && !restartFromDeformed_)
            {
                const vectorField& resetField = refCast<const vectorField>(
                    pointDisplacement->boundaryField()[patchIDs_.at(j)]);
                faceNodes -= resetField;
            }

            // Assign the (x,y,z) locations to the vertices
            // TODO: Ensure consistent order when writing/reading
            for (int i = 0; i < faceNodes.size(); i++)
            {
                for (unsigned int d = 0; d < dim_; ++d)
                {
                    vertices[verticesIndex++] = faceNodes[i][d];
                }
                if (meshConnectivity_)
                {
                    verticesMap.emplace(std::make_tuple(faceNodes[i][0], faceNodes[i][1], faceNodes[i][2]), -1);
                }
            }
        }

        // Pass the mesh vertices information to preCICE
        precice_.setMeshVertices(meshName_, vertices, vertexIDs_);

        if (meshConnectivity_)
        {
            // Build the map between OpenFOAM vertices and preCICE vertex IDs
            verticesIndex = 0;
            for (auto& key : verticesMap)
            {
                key.second = vertexIDs_[verticesIndex++];
            }

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

                // Get the list of faces and coordinates at the interface patch
                const List<face> faceField = mesh.boundaryMesh()[patchIDs_.at(j)].localFaces();
                Field<point> pointCoords = mesh.boundaryMesh()[patchIDs_.at(j)].localPoints();

                // Subtract the displacement part in case we have deformation
                if (pointDisplacement != nullptr && !restartFromDeformed_)
                {
                    const vectorField& resetField = refCast<const vectorField>(
                        pointDisplacement->boundaryField()[patchIDs_.at(j)]);
                    pointCoords -= resetField;
                }

                //Array to store the IDs we get from preCICE
                std::vector<int> triVertIDs;
                triVertIDs.reserve(faceField.size() * triaPerQuad * nodesPerTria);

                // Triangulate all faces and collect set of nodes that form triangles,
                // which are used to set mesh triangles in preCICE.
                forAll(faceField, facei)
                {
                    const face& faceQuad = faceField[facei];

                    // Triangulate the face
                    faceTriangulation faceTri(pointCoords, faceQuad, false);

                    // Iterate over all triangles generated out of each (quad) face
                    for (uint triIndex = 0; triIndex < triaPerQuad; triIndex++)
                    {
                        // Get the vertex that corresponds to the x,y,z coordinates of each node of a triangle
                        for (uint nodeIndex = 0; nodeIndex < nodesPerTria; nodeIndex++)
                        {
                            triVertIDs.push_back(verticesMap.at(std::make_tuple(pointCoords[faceTri[triIndex][nodeIndex]][0], pointCoords[faceTri[triIndex][nodeIndex]][1], pointCoords[faceTri[triIndex][nodeIndex]][2])));
                        }
                    }
                }

                DEBUG(adapterInfo("Number of triangles: " + std::to_string(faceField.size() * triaPerQuad)));

                //Set Triangles
                precice_.setMeshTriangles(meshName_, triVertIDs);
            }
        }
    }
}


void preciceAdapter::Interface::addCouplingDataWriter(
    std::string dataName,
    CouplingDataUser* couplingDataWriter)
{
    // Set the data name (from preCICE)
    couplingDataWriter->setDataName(dataName);

    // Set the patchIDs of the patches that form the interface
    couplingDataWriter->setPatchIDs(patchIDs_);

    // Set the location type in the CouplingDataUser class
    couplingDataWriter->setLocationsType(locationType_);

    // Set the location type in the CouplingDataUser class
    couplingDataWriter->checkDataLocation(meshConnectivity_);

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
    couplingDataReader->setDataName(dataName);

    // Add the CouplingDataUser to the list of readers
    couplingDataReader->setPatchIDs(patchIDs_);

    // Set the location type in the CouplingDataUser class
    couplingDataReader->setLocationsType(locationType_);

    // Check, if the current location type is supported by the data type
    couplingDataReader->checkDataLocation(meshConnectivity_);

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
    dataBuffer_.resize(dataBufferSize);
}

void preciceAdapter::Interface::readCouplingData(double relativeReadTime)
{
    // Make every coupling data reader read
    for (uint i = 0; i < couplingDataReaders_.size(); i++)
    {
        // Pointer to the current reader
        preciceAdapter::CouplingDataUser*
            couplingDataReader = couplingDataReaders_.at(i);

        // Make preCICE read vector or scalar data
        // and fill the adapter's buffer
        precice_.readData(
            meshName_,
            couplingDataReader->dataName(),
            vertexIDs_,
            relativeReadTime,
            dataBuffer_);

        // Read the received data from the buffer
        couplingDataReader->read(dataBuffer_.data(), dim_);
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
        couplingDataWriter->write(dataBuffer_.data(), meshConnectivity_, dim_);

        // Make preCICE write vector or scalar data
        precice_.writeData(
            meshName_,
            couplingDataWriter->dataName(),
            vertexIDs_,
            dataBuffer_);
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
}
