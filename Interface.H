// TODO: From Lucia, just for development

#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "CouplingDataUser/CouplingDataReader/CouplingDataReader.H"
#include "CouplingDataUser/CouplingDataWriter/CouplingDataWriter.H"
#include "precice/SolverInterface.hpp"


namespace preciceAdapter
{

class Interface
{
protected:

    //- preCICE solver interface
    precice::SolverInterface & precice_;

    //- Mesh name used in the preCICE configuration
    std::string meshName_;

    //- Mesh ID assigned by preCICE to the interface
    int meshID_;


    //- Names of the OpenFOAM patches that form the interface
    std::vector<std::string> patchNames_;

    //- OpenFOAM patches that form the interface
    std::vector<int> patchIDs_;

    //- Number of vertices of the interface
    int numDataLocations_ = 0;

    //- Vertex IDs assigned by preCICE
    int * vertexIDs_;

    //- Number of dimensions (fixed to 3, cannot handle 2)
    int numDims_ = 3;

    //- Buffer for the coupling data
    double * dataBuffer_;

    //- Vector of CouplingDataReaders
    std::vector<CouplingDataUser*> couplingDataReaders_;

    //- Vector of CouplingDataWriters
    std::vector<CouplingDataUser*> couplingDataWriters_;

    //- Extracts the locations of the face centers and exposes them to preCICE
    //  with setMeshVertices
    //  TODO: Create a mesh of nodes instead of face centers?
    void configureMesh_(const Foam::fvMesh& mesh);

public:

    //- Constructor
    Interface
    (
        precice::SolverInterface & precice,
        const Foam::fvMesh& mesh,
        std::string meshName,
        std::vector<std::string> patchNames
    );

    //- Add a CouplingDataReader to the interface
    void addCouplingDataReader
    (
        std::string dataName,
        CouplingDataUser * couplingDataReader
    );


    //- Add a CouplingDataWriter to the interface
    void addCouplingDataWriter
    (
        std::string dataName,
        CouplingDataUser * couplingDataWriter
    );

    //- Call read() on each couplingDataReader to read the coupling data
    //  from the buffer and apply the boundary conditions
    void readCouplingData();

    //- Call write() on each couplingDataWriter to extract the boundary data
    //  and write it into the buffer
    void writeCouplingData();

    //- Destructor
    ~Interface();

};

}

#endif // INTERFACE_H
