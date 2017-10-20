#ifndef CHT_H
#define CHT_H

#include "Interface.H"

#include "CHT/Temperature.H"
#include "CHT/HeatFlux.H"
#include "CHT/SinkTemperature.H"
#include "CHT/HeatTransferCoefficient.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace CHT
{

class ConjugateHeatTransfer
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Solver type
    std::string solverType_;

public:

    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

    //- Determine the solver type
    std::string determineSolverType();

    //- Configure
    void configure();

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);

};

}
}

#endif
