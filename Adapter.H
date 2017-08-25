#ifndef PRECICEADAPTER_H
#define PRECICEADAPTER_H

#include "Config.H"
#include "Interface.H"

// OpenFOAM header files
#include "Time.H"
#include "fvMesh.H"
#include "basicThermo.H"
#include "turbulenceModel.H"
#include "turbulentFluidThermoModel.H"

// Other header files
// MPI - Used to extract the MPI configuration for the preCICE initialization.
#include "mpi.h"
// preCICE Solver Interface
#include "precice/SolverInterface.hpp"

namespace preciceAdapter {

class Adapter {

private:

    //- OpenFOAM runTime object
    const Foam::Time& runTime_;

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- OpenFOAM thermophysical model
    // TODO Check the thermophysical model type
    Foam::basicThermo * thermo_ = NULL;

    //- OpenFOAM turbulence model
    // TODO Check the turbulence model type
    Foam::compressible::turbulenceModel * turbulence_ = NULL;

    //- Adapter's configuration
    Config config_;

    // Configuration parameters used in the Adapter

        //- preCICE participant name
        std::string participantName_;

        //- preCICE configuration file name
        std::string preciceConfigFilename_;

        //- Switch for subcycling
        bool subcyclingAllowed_;

        //- Switch for checkpointing
        bool checkpointingEnabled_;

        //- Type of timestep (fixed, adjustable)
        bool adjustableTimestep_;

        //- OpenFOAM Application/Solver name
        Foam::word applicationName_;

    //- Interfaces
    std::vector<Interface*> interfaces_;

    //- preCICE solver interface
    precice::SolverInterface * precice_;

public:

    //- Constructor
    Adapter(const Foam::Time& runTime, const Foam::fvMesh& mesh);

    //- Setup the adapter's configuration
    bool configure();

    //- Called by the functionObject's execute()
    void execute();

    //- Called by the functionObject's adjustTimeStep()
    void adjustTimeStep();

    //- Destructor
    ~Adapter();

};

}

#endif
