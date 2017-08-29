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

    // Timesteps

        //- Timestep dictated by preCICE
        double timestepPrecice_;

        //- Timestep used by the solver
        double timestepSolver_;

    // Checkpointing

        //- Checkpointed time (value)
        Foam::scalar couplingIterationTimeValue_;

        //- Checkpointed time (index)
        Foam::label couplingIterationTimeIndex_;

        // Vectors of pointers to the checkpointed fields and their copies

            //- Checkpointed volScalarField fields
            std::vector<Foam::volScalarField*> volScalarFields_;

            //- Checkpointed volScalarField fields (copies)
            std::vector<Foam::volScalarField*> volScalarFieldCopies_;

            //- Checkpointed volVectorField fields
            std::vector<Foam::volVectorField*> volVectorFields_;

            //- Checkpointed volVectorField fields (copies)
            std::vector<Foam::volVectorField*> volVectorFieldCopies_;

            //- Checkpointed surfaceScalarField fields
            std::vector<Foam::surfaceScalarField*> surfaceScalarFields_;

            //- Checkpointed surfaceScalarField fields (copies)
            std::vector<Foam::surfaceScalarField*> surfaceScalarFieldCopies_;

    //- Printed output and error handling
    void adapterInfo(const std::string message, const std::string level);

    // Methods communicating with preCICE

        //- Initialize preCICE
        void initialize();

        //- Advance preCICE
        void advance();

        //- Read the coupling data at each interface
        void readCouplingData();

        //- Write the coupling data at each interface
        void writeCouplingData();

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

    // Methods for checkpointing

        //- Make a copy of the runTime object
        void storeCheckpointTime();

        //- Restore the copy of the runTime object
        void reloadCheckpointTime();

        // Add checkpoint fields, depending on the type

            //- Add a volScalarField to checkpoint
            void addCheckpointField( volScalarField & field );

            //- Add a volVectorField to checkpoint
            void addCheckpointField( volVectorField & field );

            //- Add a surfaceScalarField to checkpoint
            void addCheckpointField( surfaceScalarField & field );

            //- TODO Add a surfaceVectorField to checkpoint
            // TODO Make all these a template?

        //- Read the checkpoint - restore the fields and time
        void readCheckpoint();

        //- Write the checkpoint - store the fields and time
        void writeCheckpoint();

public:

    // Methods called by the functionObject

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
