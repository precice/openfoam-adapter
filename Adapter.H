#ifndef PRECICEADAPTER_H
#define PRECICEADAPTER_H

#include "Config.H"

// OpenFOAM header files
#include "Time.H"

// Other header files
// MPI - Used to extract the MPI configuration for the preCICE initialization.
#include "mpi.h"
// preCICE Solver Interface
#include "precice/SolverInterface.hpp"

namespace preciceAdapter {

class Adapter {

private:

    //- runTime object
    const Foam::Time& runTime_;

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

    //- preCICE solver interface
    precice::SolverInterface * precice_;

public:

    //- Constructor
    Adapter(const Foam::Time& runTime);

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
