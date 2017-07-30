#include "Adapter.H"

#include "IOstreams.H"

preciceAdapter::Adapter::Adapter(const Foam::Time& runTime)
:
runTime_(runTime)
{
    Foam::Info << "Entered the Adapter() constructor." << Foam::nl;
    return;
}

bool preciceAdapter::Adapter::configure()
{
    Foam::Info << "Entered Adapter::configure()." << Foam::nl;

    config_ = preciceAdapter::Config();

    // Read the adapter's configuration file
    if ( !config_.configFileRead() )
    {
        runTime_.stopAt(Foam::Time::saNoWriteNow);
        FatalErrorIn("configFileCheck()")
            << "Error in the preCICE adapter:"
            << Foam::nl
            << "There was a problem reading the adapter's configuration file. "
            << "See the log for details."
            << Foam::nl
            << Foam::exit(Foam::FatalError);
        return false;
    }

    // Copy the variables from the config_
    participantName_ = config_.participantName();
    preciceConfigFilename_ = config_.preciceConfigFilename();
    subcyclingAllowed_ = config_.subcyclingAllowed();
    checkpointingEnabled_ = config_.checkpointingEnabled();

    // Check the timestep type (fixed vs adjustable)
    Foam::Info << "---[preciceAdapter] Check the timestep type (fixed vs adjustable)." << Foam::nl;
    adjustableTimestep_ = runTime_.controlDict().lookupOrDefault("adjustTimeStep", false);

    if ( adjustableTimestep_ ) {
        Foam::Info << "---[preciceAdapter]   Timestep type: adjustable." << Foam::nl;
    } else {
        Foam::Info << "---[preciceAdapter]   Timestep type: fixed." << Foam::nl;
    }

    // Add fields in the checkpointing list
    if ( checkpointingEnabled_ ) {
        Foam::Info << "---[preciceAdapter] Add checkpoint fields (decide which)." << Foam::nl;
    }

    // Initialize preCICE
    Foam::Info << "---[preciceAdapter] Create the preCICE solver interface." << Foam::nl;
    int MPIEnabled = 0;
    int MPIRank = 0;
    int MPISize = 1;

    MPI_Initialized( &MPIEnabled );
    Foam::Info << "---[preciceAdapter]   MPI used: " << MPIEnabled << Foam::nl;

    if ( MPIEnabled )
    {
        MPI_Comm_rank( MPI_COMM_WORLD, &MPIRank );
        Foam::Info << "---[preciceAdapter]   MPI rank: " << MPIRank << Foam::nl;

        MPI_Comm_size( MPI_COMM_WORLD, &MPISize );
        Foam::Info << "---[preciceAdapter]   MPI size: " << MPISize << Foam::nl;
    }

    precice_ = new precice::SolverInterface( participantName_, MPIRank, MPISize );
    Foam::Info << "---[preciceAdapter]   preCICE solver interface was created." << Foam::nl;

    Foam::Info << "---[preciceAdapter] Configure preCICE." << Foam::nl;
    precice_->configure( preciceConfigFilename_ );

    Foam::Info << "---[preciceAdapter] Write coupling data (for the first iteration)" << Foam::nl;
    Foam::Info << "---[preciceAdapter] Initialize preCICE data." << Foam::nl;
    Foam::Info << "---[preciceAdapter] ---" << Foam::nl;

    Foam::Info << "---[preciceAdapter] Read coupling data (for the first iteration)" << Foam::nl;

    // Write checkpoint (for the first iteration)
    if (checkpointingEnabled_) {
        Foam::Info << "---[preciceAdapter] Write checkpoint (for the first iteration)" << Foam::nl;
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        Foam::Info << "---[preciceAdapter] Adjust the solver's timestep (if fixed timestep, for the first iteration)" << Foam::nl;
    }

    return true;
}

void preciceAdapter::Adapter::execute()
{
    Foam::Info << "---[preciceAdapter] if (coupling ongoing) {" << Foam::nl;
    Foam::Info << "---[preciceAdapter]   Write coupling data (from the previous iteration)." << Foam::nl;
    Foam::Info << "---[preciceAdapter]   Advance preCICE (from the previous iteration)." << Foam::nl;
    Foam::Info << "---[preciceAdapter]   Read coupling data (from the previous iteration)." << Foam::nl;

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_) {
        Foam::Info << "---[preciceAdapter]   Adjust the solver's timestep (if fixed timestep, from the previous iteration)." << Foam::nl;
    }

    // Read checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Foam::Info << "---[preciceAdapter]   Read checkpoint (from the previous iteration)." << Foam::nl;
    }

    // Write checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Foam::Info << "---[preciceAdapter]   Write checkpoint (from the previous iteration)." << Foam::nl;
    }

    Foam::Info << "---[preciceAdapter]   Write if coupling timestep complete (?)." << Foam::nl;
    Foam::Info << "---[preciceAdapter] } else {" << Foam::nl;
    Foam::Info << "---[preciceAdapter]   Exit the loop." << Foam::nl;
    Foam::Info << "---[preciceAdapter] }" << Foam::nl;

    return;
}

void preciceAdapter::Adapter::adjustTimeStep()
{
    Foam::Info << "---[preciceAdapter] Adjust the solver's timestep (only if dynamic timestep is used)." << Foam::nl;
    return;
}

preciceAdapter::Adapter::~Adapter()
{
    Foam::Info << "---[preciceAdapter] Destroy the preCICE Solver Interface." << Foam::nl;

    // TODO: throws segmentation fault if it has not been initialized at premature exit.
    // delete precice_;
}
