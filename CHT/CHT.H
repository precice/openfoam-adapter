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

    //- Name of the transportProperties dictionary
    std::string nameTransportProperties_;

    //- Name of the temperature field
    std::string nameT_;

public:

    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

    //- Determine the solver type
    std::string determineSolverType();

    //- Configure
    void configure(
        const std::string nameTransportProperties,
        const std::string nameT
    );

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);

};

}
}

#endif
