#ifndef FF_H
#define FF_H

#include "Interface.H"

#include "FF/Velocity.H"
#include "FF/Pressure.H"
#include "FF/PressureGradient.H"
#include "FF/VelocityGradient.H"

namespace preciceAdapter
{
namespace FF
{

class FluidFluid
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // TODO: Create a struct for all the parameter names

    //- Solver type
    std::string solverType_ = "none";

    //- Name of the velocity field
    std::string nameU_ = "U";

    //- Name of the pressure field
    std::string nameP_ = "p";

    //- Determine the solver type
    std::string determineSolverType();

    //- Read the FF-related options from the adapter's configuration file
    bool readConfig(const Foam::IOdictionary& adapterConfig);

public:
    //- Constructor
    FluidFluid(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const Foam::IOdictionary& adapterConfig);

    //- Add coupling data writers
    bool addWriters(std::string dataName, Interface* interface);

    //- Add coupling data readers
    bool addReaders(std::string dataName, Interface* interface);
};

}
}

#endif
