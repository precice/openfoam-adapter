#ifndef CHT_H
#define CHT_H

#include "Interface.H"

#include "CHT/Temperature.H"
#include "CHT/HeatFlux.H"
#include "CHT/SinkTemperature.H"
#include "CHT/HeatTransferCoefficient.H"

#include "fvCFD.H"

// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"

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
    std::string solverType_ = "none";

    //- Name of the transportProperties dictionary
    std::string nameTransportProperties_ = "transportProperties";

    //- Name of the temperature field
    std::string nameT_ = "T";

    //- Name of the thermal conductivity for a basic solver
    std::string nameKappa_ = "k";

    //- Name of the density for an incompressible solver
    std::string nameRho_ = "rho";

    //- Name of the specific heat for an incompressible solver
    std::string nameCp_ = "Cp";

    //- Name of the Prandtl number for an incompressible solver
    std::string namePr_ = "Pr";

    //- Name of the turbulent thermal diffusivity field for an incompressible solver
    std::string nameAlphat_ = "alphat";

protected:

    //- Determine the solver type
    std::string determineSolverType();

    //- Read the CHT-related options from the adapter's configuraiton file
    bool readConfig(const YAML::Node adapterConfig);

public:

    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

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
