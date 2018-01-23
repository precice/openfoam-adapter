#ifndef FLUIDS_H
#define FLUIDS_H

#include "Interface.H"

#include "Fluids/Pressure.H"
#include "Fluids/Velocity.H"

#include "fvCFD.H"

// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"

namespace preciceAdapter
{
namespace Fluids
{

class VelocityAndPressure
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // TODO: Create a struct for all the parameter names

    //- Solver type
    std::string solverType_ = "none";

    //- Name of the transportProperties dictionary
    std::string nameTransportProperties_ = "transportProperties";

    //- Name of the pressure field
    std::string nameP_ = "P";

    //- Name of the velocity field
    std::string nameU_ = "U";

    //- Name of the density for an incompressible solver
    std::string nameRho_ = "rho";

    // //- Name of the heat capacity for an incompressible solver
    // std::string nameCp_ = "Cp";

    // //- Name of the Prandtl number for an incompressible solver
    // std::string namePr_ = "Pr";

    // //- Name of the turbulent thermal diffusivity field for an incompressible solver
    // std::string nameAlphat_ = "alphat";

protected:

    //- Determine the solver type
    std::string determineSolverType();

    //- Read the CHT-related options from the adapter's configuraiton file
    bool readConfig(const YAML::Node adapterConfig);

public:

    //- Constructor
    VelocityAndPressure(const Foam::fvMesh& mesh);

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
