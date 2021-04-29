#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "CouplingDataUser.H"
#include "precice/SolverInterface.hpp"


namespace preciceAdapter
{

class Interface
{
protected:
    //- preCICE solver interface
    precice::SolverInterface& precice_;

    //- Mesh name used in the preCICE configuration
    std::string meshName_;

    //- Point locations type (faceCenters vs faceNodes)
    std::string locationsType_;

    //- Mesh ID assigned by preCICE to the interface
    int meshID_;

    //- Names of the OpenFOAM patches that form the interface
    std::vector<std::string> patchNames_;

    //- OpenFOAM patches that form the interface
    std::vector<int> patchIDs_;

    //- Number of data points (cell centers) on the interface
    int numDataLocations_ = 0;

    //- Vertex IDs assigned by preCICE
    int* vertexIDs_;

    //- Buffer for the coupling data
    double* dataBuffer_;

    //- Vector of CouplingDataReaders
    std::vector<CouplingDataUser*> couplingDataReaders_;

    //- Vector of CouplingDataWriters
    std::vector<CouplingDataUser*> couplingDataWriters_;

    //Switch for faceTriangulation (nearest projection)
    bool meshConnectivity_;

    // Simulation dimension
    unsigned int dim_;

    //- Extracts the locations of the face centers or face nodes
    //  and exposes them to preCICE with setMeshVertices
    void configureMesh(const Foam::fvMesh& mesh);

public:
    //- Constructor
    Interface(
        precice::SolverInterface& precice,
        const Foam::fvMesh& mesh,
        std::string meshName,
        std::string locationsType,
        std::vector<std::string> patchNames,
        bool meshConnectivity);

    //- Add a CouplingDataUser to read data from the interface
    void addCouplingDataReader(
        std::string dataName,
        CouplingDataUser* couplingDataReader);


    //- Add a CouplingDataUser to write data on the interface
    void addCouplingDataWriter(
        std::string dataName,
        CouplingDataUser* couplingDataWriter);

    //- Allocate an appropriate buffer for scalar or vector data.
    //  If at least one couplingDataUser has vector data, then
    //  define a buffer for 3D data. Otherwise, for 1D data.
    void createBuffer();

    //- Call read() on each registered couplingDataReader to read the coupling
    //  data from the buffer and apply the boundary conditions
    void readCouplingData();

    //- Call write() on each registered couplingDataWriter to extract the boundary
    //  data and write them into the buffer
    void writeCouplingData();

    //- Destructor
    ~Interface();
};

}

#endif
