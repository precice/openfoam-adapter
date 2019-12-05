#ifndef AITS_H
#define AITS_H

#include "Interface.H"

#include "AITS/TemperatureAIT.H"

#include "fvCFD.H"

// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"

namespace preciceAdapter
{
namespace AITS
{

class AITSolver
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // TODO: Create a struct for all the parameter names

    //- Solver type
    std::string solverType_ = "none";

    //- Name of the transportProperties dictionary
    std::string nameTransportProperties_ = "transportProperties";

    //- Name of the temperature field
    std::string nameT_ = "T";


protected:

    //- Determine the solver type
    std::string determineSolverType();

    //- Read the AITS-related options from the adapter's configuraiton file
    bool readConfig(const YAML::Node adapterConfig);

public:

    //- Constructor
    AITSolver(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const YAML::Node adapterConfig);

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);
};

}
}

#endif
