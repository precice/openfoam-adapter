#ifndef FF_H
#define FF_H

#include "Interface.H"

#include "FF/Velocity.H"
#include "FF/Pressure.H"
#include "FF/Temperature.H"
#include "FF/Alpha.H"
#include "FF/PressureGradient.H"
#include "FF/VelocityGradient.H"
#include "FF/TemperatureGradient.H"
#include "FF/AlphaGradient.H"
#include "FF/Phi.H"

#include "fvCFD.H"

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

    //- Name of the temperature field
    std::string nameT_ = "T";

    //- Name of the phase variable (alpha) field
    std::string nameAlpha_ = "alpha";

    //- Name of the face flux field
    std::string namePhi_ = "phi";

    //- Flux correction of velocity
    bool fluxCorrection_ = false;

    //- Determine the solver type
    std::string determineSolverType();

    //- Read the FF-related options from the adapter's configuration file
    bool readConfig(const IOdictionary& adapterConfig);

public:
    //- Constructor
    FluidFluid(const Foam::fvMesh& mesh);

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
