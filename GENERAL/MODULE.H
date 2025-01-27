#ifndef GENERALMODULE_H
#define GENERALMODULE_H

#include "Interface.H"

#include "GENERAL/HeatTransfer.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace MODULE
{

class ConjugateHeatTransfer
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Name of the temperature field
    std::string nameT_ = "T";

protected:
    //- Determine the solver type
    std::string determineSolverType();

    //- Read the Module related options from the adapter's configuration file
    bool readConfig(const IOdictionary& adapterConfig);

public:
    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const IOdictionary& adapterConfig);

    //- Add coupling data writers
    bool addWriters(std::string dataName, Interface* interface);

    //- Add coupling data readers
    bool addReaders(std::string dataName, Interface* interface);
};

}
}

#endif
