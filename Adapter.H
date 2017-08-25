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

    //- Timestep dictated by preCICE
    double timestepPrecice_;

    //- Timestep used by the solver
    double timestepSolver_;

    //- Initialize preCICE
    void initialize();

    //- Read the coupling data at each interface
    void readCouplingData();

    //- Write the coupling data at each interface
    void writeCouplingData();

    //- Advance preCICE
    void advance();

    //- Adjust the timestep of the solver according to preCICE
    void adjustSolverTimeStep();

    //- Determine if the coupling is still happening
    bool isCouplingOngoing();

    //- Determine if the coupling timestep has been completed
    bool isCouplingTimestepComplete();

    //- Determine if a checkpoint must be read
    bool isReadCheckpointRequired();

    //- Determine if a checkpoint must be written
    bool isWriteCheckpointRequired();

    //- Tell preCICE that the checkpoint has been read
    void fulfilledReadCheckpoint();

    //- Tell preCICE that the checkpoint has been written
    void fulfilledWriteCheckpoint();

    //- TODO Set checkpointing on/off
    // void setCheckpointingEnabled( bool enabled );

    //- TODO Determine if the checkpointing is enabled
    // bool isCheckpointingEnabled();

    // TODO Methods for adding checkpointed fields
    // TODO Methods for reading/writing checkpoints

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
