#include "Adapter.H"
#include "Interface.H"

#include "IOstreams.H"

#include "CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "CouplingDataUser/CouplingDataReader/HeatFluxBoundaryCondition.h"
#include "CouplingDataUser/CouplingDataReader/BuoyantPimpleHeatFluxBoundaryCondition.h"

#include "CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "CouplingDataUser/CouplingDataWriter/HeatFluxBoundaryValues.h"
#include "CouplingDataUser/CouplingDataWriter/BuoyantPimpleHeatFluxBoundaryValues.h"

using namespace Foam;

#define ADAPTER_DEBUG_MODE 1

preciceAdapter::Adapter::Adapter(const Time& runTime, const fvMesh& mesh)
:
runTime_(runTime),
mesh_(mesh),
timestepSolver_(-1)
{
    adapterInfo( "Entered the Adapter() constructor.", "debug" );
    return;
}

void preciceAdapter::Adapter::adapterInfo(const std::string message, const std::string level = "info")
{
    if ( level.compare("info") == 0 )
    {
        // Prepend the message with a string
        Info << "---[preciceAdapter] "
             << message.c_str()
             << nl;
    }
    else if ( level.compare("warning") == 0 )
    {
        // Produce a warning message with cyan header
        FatalErrorInFunction
             << "\033[36m" // cyan color
             << "Error in the preCICE adapter: "
             << nl
             << "\033[0m" // restore color
             << message.c_str()
             << nl;
    }
    else if ( level.compare("error") == 0 )
    {
        // Produce an error message with red header
        // and exit the functionObject
        FatalErrorInFunction
             << "\033[31m" // red color
             << "Error in the preCICE adapter: "
             << nl
             << "\033[0m" // restore color
             << message.c_str()
             << exit(FatalError);
    }
    else if ( level.compare("error-critical") == 0 )
    {
        // Stop the simulation: when a functionObject produces an error,
        // OpenFOAM catches it as a warning and the simulation continues.
        // This forces the simulation to stop.
        runTime_.stopAt(Time::saNoWriteNow);

        // Produce an error message with red header
        // and exit the functionObject
        FatalErrorInFunction
             << "\033[31m" // red color
             << "Critical Error in the preCICE adapter: "
             << nl
             << "\033[0m" // restore color
             << message.c_str()
             << exit(FatalError);
    }
    else if ( level.compare("debug") == 0 )
    {
        if ( ADAPTER_DEBUG_MODE )
        {
            Info << "---[preciceAdapter] [DEBUG] "
            << message.c_str()
            << nl;
        }
    }
    else if ( level.compare("dev") == 0 )
    {
        Info << "\033[36m" // cyan color
             << "---[preciceAdapter] [under development] "
             << "\033[0m" // restore color
             << message.c_str()
             << nl;
    }
    else
    {
        Info << "---[preciceAdapter] [unknown info level] "
             << message.c_str()
             << nl;
    }

    return;
}

bool preciceAdapter::Adapter::configure()
{
    adapterInfo( "Entered Adapter::configure().", "debug" );

    config_ = preciceAdapter::Config();

    // Read the adapter's configuration file
    if ( !config_.configFileRead( runTime_.path() ) )
    {
        adapterInfo( "There was a problem reading the adapter's configuration file. See the log for details", "error-critical" );
        return false;
    }

    // Copy the variables from the config_
    participantName_ = config_.participantName();
    preciceConfigFilename_ = config_.preciceConfigFilename();
    subcyclingAllowed_ = config_.subcyclingAllowed();
    checkpointingEnabled_ = config_.checkpointingEnabled();

    // Get the solver name
    runTime_.controlDict().readIfPresent("application", applicationName_);
    adapterInfo( "Application: " + applicationName_, "debug" );

    // Check the timestep type (fixed vs adjustable)
    adapterInfo( "Checking the timestep type (fixed vs adjustable)...", "debug" );
    adjustableTimestep_ = runTime_.controlDict().lookupOrDefault("adjustTimeStep", false);

    if ( adjustableTimestep_ ) {
        adapterInfo( "  Timestep type: adjustable.", "debug" );
    } else {
        adapterInfo( "  Timestep type: fixed.", "debug" );
    }

    // Add fields in the checkpointing list
    if ( checkpointingEnabled_ )
    {
        adapterInfo( "Add checkpoint fields", "dev" );
    }

    // Initialize preCICE
    adapterInfo( "Creating the preCICE solver interface...", "debug" );
    int MPIEnabled = 0;
    int MPIRank = 0;
    int MPISize = 1;

    MPI_Initialized( &MPIEnabled );
    adapterInfo( "  MPI used: " + MPIEnabled, "debug" );

    if ( MPIEnabled )
    {
        MPI_Comm_rank( MPI_COMM_WORLD, &MPIRank );
        adapterInfo( "  MPI rank: " + MPIRank, "debug" );

        MPI_Comm_size( MPI_COMM_WORLD, &MPISize );
        adapterInfo( "  MPI size: " + MPISize, "debug" );
    }

    precice_ = new precice::SolverInterface( participantName_, MPIRank, MPISize );
    adapterInfo( "  preCICE solver interface was created.", "debug" );

    adapterInfo( "Configuring preCICE...", "debug" );
    precice_->configure( preciceConfigFilename_ );
    adapterInfo( "  preCICE was configured.", "debug" );

    // Get the thermophysical model
    adapterInfo( "Specifying the thermophysical model...", "debug" );
    if (mesh_.foundObject<basicThermo>("thermophysicalProperties")) {
        adapterInfo( "  - Found 'thermophysicalProperties', refering to 'basicThermo'.", "debug" );
        thermo_ = const_cast<basicThermo*>(&mesh_.lookupObject<basicThermo>("thermophysicalProperties"));
    } else {
        adapterInfo( "  - Did not find 'thermophysicalProperties', no thermoModel specified.", "debug" );
        if (mesh_.foundObject<volScalarField>("T")) {
            adapterInfo( "  - Found the T object.", "debug" );
        }
    }

    // Get the turbulence model
    adapterInfo( "Specifying the turbulence model...", "debug" );
    if (mesh_.foundObject<compressible::turbulenceModel>("turbulenceProperties")) {
        adapterInfo( "  - Found 'turbulenceProperties', refering to 'compressible::turbulenceModel'.", "debug" );
        turbulence_ = const_cast<compressible::turbulenceModel*>(&mesh_.lookupObject<compressible::turbulenceModel>("turbulenceProperties"));
    } else {
        adapterInfo( "  - Did not find 'turbulenceProperties', no turbulenceModel specified.", "debug" );
    }

    // Create interfaces
    adapterInfo( "Creating interfaces...", "debug" );
    for ( uint i = 0; i < config_.interfaces().size(); i++ )
    {
        Interface * interface = new Interface( *precice_, mesh_, config_.interfaces().at( i ).meshName, config_.interfaces().at( i ).patchNames );
        interfaces_.push_back( interface );
        adapterInfo( "Interface created on mesh" + config_.interfaces().at( i ).meshName, "debug" );

        // TODO: Add coupling data users for 'sink temperature' and for 'heat transfer coefficient'
        adapterInfo( "Adding coupling data writers...", "dev" );
        for ( uint j = 0; j < config_.interfaces().at( i ).writeData.size(); j++ )
        {
            std::string dataName = config_.interfaces().at( i ).writeData.at( j );

            if ( dataName.compare( "Temperature" ) == 0 )
            {
                if (thermo_) {
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( &thermo_->T() );
                    interface->addCouplingDataWriter( dataName, bw );
                    adapterInfo( "  Added Temperature from thermoModel.", "dev" );
                } else {
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) ); // TODO Mesh does not have T()
                    interface->addCouplingDataWriter( dataName, bw );
                    adapterInfo( "  Added Temperature from T.", "dev" );
                }
            }

            if ( dataName.compare( "Heat-Flux" ) == 0 )
            {
                if ( applicationName_.compare( "buoyantPimpleFoam" ) == 0 )
                {
                    if ( thermo_ && turbulence_ ) {
                        BuoyantPimpleHeatFluxBoundaryValues * bw = new BuoyantPimpleHeatFluxBoundaryValues( &thermo_->T(), thermo_, turbulence_ );
                        interface->addCouplingDataWriter( dataName, bw );
                        adapterInfo( "  Added Heat Flux for buoyantPimpleFoam", "dev" );
                    } else {
                        adapterInfo( "  Problem: no thermo or no turbulence model specified", "error" );
                    }
                }
                else
                {
                    if ( thermo_ ) {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( &thermo_->T(), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        adapterInfo( "  Added Heat Flux with temperature from thermoModel.", "dev" );
                        adapterInfo( "  Assuming conductivity k = 100.", "dev" );
                    } else {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        adapterInfo( "  Added Heat Flux with temperature from T.", "dev" );
                        adapterInfo( "  Assuming conductivity k = 100.", "dev" );
                    }
                }
            }

            if ( dataName.compare( "Heat-Transfer-Coefficient" ) == 0 )
            {
                // TODO Implement Heat Transfer Coefficient
                adapterInfo( "  Heat Transfer Coefficient not implemented yet!", "error" );
            }

            if ( dataName.compare( "Sink-Temperature" ) == 0 )
            {
                // TODO Implement Sink Temperature
                adapterInfo( "  Sink Temperature not implemented yet!", "error" );
            }

        } // end add coupling data writers

        adapterInfo( "Adding coupling data readers...", "dev" );
        for ( uint j = 0; j < config_.interfaces().at( i ).readData.size(); j++ )
        {
            std::string dataName = config_.interfaces().at( i ).readData.at( j );

            if ( dataName.compare( "Temperature" ) == 0 )
            {
                if ( thermo_ ) {
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( &thermo_->T() );
                    interface->addCouplingDataReader( dataName, br );
                    adapterInfo( "  Added Temperature from thermoModel.", "dev" );
                } else {
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) );
                    interface->addCouplingDataReader( dataName, br );
                    adapterInfo( "  Added Temperature from T.", "dev" );
                }
            }

            if ( dataName.compare( "Heat-Flux" ) == 0 )
            {
            	if ( applicationName_.compare( "buoyantPimpleFoam" ) == 0 )
            	{
                    if ( thermo_ && turbulence_ ) {
                        BuoyantPimpleHeatFluxBoundaryCondition * br = new BuoyantPimpleHeatFluxBoundaryCondition( &thermo_->T(), thermo_, turbulence_ );
                        interface->addCouplingDataReader( dataName, br );
                        adapterInfo( "  Added Heat Flux for buoyantPimpleFoam", "dev" );
                    } else {
                        adapterInfo( "  Problem: no thermo or no turbulence model specified", "error" );
                    }
                }
                else
                {
                    if ( thermo_ ) {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( &thermo_->T(), k);
                        interface->addCouplingDataReader( dataName, br );
                        adapterInfo( "  Added Heat Flux with temperature from thermoModel.", "dev" );
                        adapterInfo( "  Assuming conductivity k = 100.", "dev" );
                    } else {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataReader( dataName, br );
                        adapterInfo( "  Added Heat Flux with temperature from T.", "dev" );
                        adapterInfo( "  Assuming conductivity k = 100.", "dev" );
                    }
            	}
            }

            if ( dataName.compare( "Heat-Transfer-Coefficient" ) == 0 )
            {
                // TODO Implement Heat Transfer Coefficient
                adapterInfo( "  Heat Transfer Coefficient not implemented yet!", "error" );
            }

            if ( dataName.compare( "Sink-Temperature" ) == 0 )
            {
                // TODO Implement Sink Temperature
                adapterInfo( "  Sink Temperature not implemented yet!", "error" );
            }

        } // end add coupling data readers
    }

    adapterInfo( "Writing coupling data (for the first iteration) & initializing preCICE data...", "debug");
    initialize();

    adapterInfo( "preCICE was configured and initialized", "info" );

    adapterInfo( "Reading coupling data (for the first iteration)...", "debug" );
    readCouplingData();

    // Write checkpoint (for the first iteration)
    if ( isWriteCheckpointRequired() )
    {
        if ( checkpointingEnabled_ )
        {
            adapterInfo( "Writing a checkpoint (for the first iteration)...", "debug" );
            writeCheckpoint();
            fulfilledWriteCheckpoint();
            adapterInfo( "  Checkpoint was written.", "debug" );
        }
        else
        {
            adapterInfo("Checkpointing is not enabled.", "error-critical");
        }
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        adapterInfo( "Adjusting the solver's timestep (if fixed timestep, for the second iteration)", "dev" );
        adjustSolverTimeStep();
    }

    return true;
}

void preciceAdapter::Adapter::execute()
{
    if ( isCouplingOngoing() ) {
        adapterInfo( "The coupling is ongoing.", "info" );

        adapterInfo( "Writing coupling data (from the previous iteration)...", "debug" );
        writeCouplingData();

        adapterInfo( "Advancing preCICE...", "info" );
        advance();

        adapterInfo( "Reading coupling data (from the previous iteration)...", "debug" );
        readCouplingData();

        // Adjust the timestep, if it is fixed
        if (!adjustableTimestep_) {
            adapterInfo( "Adjusting the solver's timestep... (if fixed timestep, from the previous iteration)", "dev" );
            adjustSolverTimeStep();
        }

        // Read checkpoint (from the previous iteration)
        if ( isReadCheckpointRequired() )
        {
            if ( checkpointingEnabled_ )
            {
                adapterInfo( "Reading a checkpoint... (from the previous iteration)", "debug" );
                readCheckpoint();
                fulfilledReadCheckpoint();
                adapterInfo( "  Checkpoint was read.", "debug" );
            }
            else
            {
                adapterInfo( "Checkpointing is not enabled.", "critical-error" );
            }
        }

        // Write checkpoint (from the previous iteration)
        if ( isWriteCheckpointRequired() )
        {
            if ( checkpointingEnabled_ )
            {
                adapterInfo( "Writing a checkpoint... (from the previous iteration)", "debug" );
                writeCheckpoint();
                fulfilledWriteCheckpoint();
                adapterInfo( "  Checkpoint was written", "debug" );
            }
            else
            {
                adapterInfo( "Checkpointing is not enabled.", "critical-error" );
            }
        }

        if (isCouplingTimestepComplete()) {
            adapterInfo( "The coupling timestep is complete.", "info" );
            adapterInfo( "Writing the results...", "dev" );
            // TODO write() or writeNow()?
            // TODO Check if results are written at the last timestep.
            // TODO Check for implicit coupling to avoid multiple writes.
            const_cast<Time&>(runTime_).writeNow();
        }
    }
    else
    {
        adapterInfo( "The coupling completed.", "info" );
        // TODO finalize preCICE and remove from destructor
    }

    return;
}

void preciceAdapter::Adapter::adjustTimeStep()
{
    adapterInfo( "Adjust the solver's timestep (only if dynamic timestep is used)", "debug" );
    adjustSolverTimeStep();

    return;
}

void preciceAdapter::Adapter::readCouplingData()
{
    for ( uint i = 0; i < interfaces_.size(); i++ )
    {
        interfaces_.at( i )->readCouplingData();
    }
}

void preciceAdapter::Adapter::writeCouplingData()
{
    for ( uint i = 0; i < interfaces_.size(); i++ )
    {
        interfaces_.at( i )->writeCouplingData();
    }
}

void preciceAdapter::Adapter::initialize()
{
    timestepPrecice_ = precice_->initialize();

    if ( precice_->isActionRequired( precice::constants::actionWriteInitialData() ) )
    {
        writeCouplingData();
        precice_->fulfilledAction( precice::constants::actionWriteInitialData() );
    }

    precice_->initializeData();
}

void preciceAdapter::Adapter::advance()
{
    if ( timestepSolver_ == -1 )
    {
        timestepPrecice_ = precice_->advance( timestepPrecice_ );
    }
    else
    {
        timestepPrecice_ = precice_->advance( timestepSolver_ );
    }
}

void preciceAdapter::Adapter::adjustSolverTimeStep()
{
    double timestepSolverDetermined = runTime_.deltaT().value();

    if ( timestepSolverDetermined < timestepPrecice_ )
    {
        if ( !subcyclingAllowed_ )
        {
            // TODO This case doesn't make sense - drop the option
            adapterInfo(
                "The solver's timestep cannot be smaller than the "
                "coupling timestep, because subcycling has not been activated. "
                "Forcing the solver to use the coupling timestep.",
                "warning"
            );
            timestepSolver_ = timestepPrecice_;
        }
        else
        {
            adapterInfo(
                "The solver's timestep is smaller than the "
                "coupling timestep. Subcycling...",
                "warning"
            );
            timestepSolver_ = timestepSolverDetermined;
        }
    }
    else if ( timestepSolverDetermined > timestepPrecice_ )
    {
        adapterInfo(
            "The solver's timestep cannot be larger than the coupling timestep."
            " Adjusting from " +
            std::to_string(timestepSolverDetermined) +
            " to " +
            std::to_string(timestepPrecice_),
            "warning"
        );
        timestepSolver_ = timestepPrecice_;
    }
    else
    {
        // TODO Merge?
        adapterInfo(
            "The solver's timestep is the same as the coupling timestep.",
            "info"
        );
        timestepSolver_ = timestepPrecice_;
    }

    // Update the solver's timestep (but don't trigger the adjustDeltaT(),
    // which also triggers the functionObject's adjustTimeStep()) (TODO)
    const_cast<Time&>(runTime_).setDeltaT( timestepSolver_, false );
}

bool preciceAdapter::Adapter::isCouplingOngoing()
{
    return precice_->isCouplingOngoing();
}

bool preciceAdapter::Adapter::isCouplingTimestepComplete()
{
    return precice_->isTimestepComplete();
}

bool preciceAdapter::Adapter::isReadCheckpointRequired()
{
    return precice_->isActionRequired( precice::constants::actionReadIterationCheckpoint() );
}

bool preciceAdapter::Adapter::isWriteCheckpointRequired()
{
    return precice_->isActionRequired( precice::constants::actionWriteIterationCheckpoint() );
}

void preciceAdapter::Adapter::fulfilledReadCheckpoint()
{
    precice_->fulfilledAction( precice::constants::actionReadIterationCheckpoint() );
}

void preciceAdapter::Adapter::fulfilledWriteCheckpoint()
{
    precice_->fulfilledAction( precice::constants::actionWriteIterationCheckpoint() );
}

void preciceAdapter::Adapter::storeCheckpointTime()
{
    couplingIterationTimeIndex_ = runTime_.timeIndex();
    couplingIterationTimeValue_ = runTime_.value();
}

void preciceAdapter::Adapter::reloadCheckpointTime()
{
    const_cast<Time&>(runTime_).setTime( couplingIterationTimeValue_, couplingIterationTimeIndex_ );
}

void preciceAdapter::Adapter::addCheckpointField( volScalarField & field )
{
    volScalarField * copy = new volScalarField( field );
    volScalarFields_.push_back( &field );
    volScalarFieldCopies_.push_back( copy );
}

void preciceAdapter::Adapter::addCheckpointField( volVectorField & field )
{
    volVectorField * copy = new volVectorField( field );
    volVectorFields_.push_back( &field );
    volVectorFieldCopies_.push_back( copy );
}

void preciceAdapter::Adapter::addCheckpointField( surfaceScalarField & field )
{
    surfaceScalarField * copy = new surfaceScalarField( field );
    surfaceScalarFields_.push_back( &field );
    surfaceScalarFieldCopies_.push_back( copy );
}

void preciceAdapter::Adapter::readCheckpoint()
{
    // Reload the runTime
    reloadCheckpointTime();

    // Reload all the fields of type volScalarField
    for ( uint i = 0 ; i < volScalarFields_.size() ; i++ )
    {
        *( volScalarFields_.at( i ) ) == *( volScalarFieldCopies_.at( i ) );
    }

    // Reload all the fields of type volVectorField
    for ( uint i = 0 ; i < volVectorFields_.size() ; i++ )
    {
        *( volVectorFields_.at( i ) ) == *( volVectorFieldCopies_.at( i ) );
    }

    // Reload all the fields of type surfaceScalarField
    for ( uint i = 0 ; i < surfaceScalarFields_.size() ; i++ )
    {
        *( surfaceScalarFields_.at( i ) ) == *( surfaceScalarFieldCopies_.at( i ) );
    }

    // TODO Reload all the fields of type surfaceVectorField
}

void preciceAdapter::Adapter::writeCheckpoint()
{
    // Store the runTime
    storeCheckpointTime();

    // Store all the fields of type volScalarField
    for ( uint i = 0 ; i < volScalarFields_.size() ; i++ )
    {
        *( volScalarFieldCopies_.at( i ) ) == *( volScalarFields_.at( i ) );
    }

    // Store all the fields of type volVectorField
    for ( uint i = 0 ; i < volVectorFields_.size() ; i++ )
    {
        *( volVectorFieldCopies_.at( i ) ) == *( volVectorFields_.at( i ) );
    }

    // Store all the fields of type surfaceScalarField
    for ( uint i = 0 ; i < surfaceScalarFields_.size() ; i++ )
    {
        *( surfaceScalarFieldCopies_.at( i ) ) == *( surfaceScalarFields_.at( i ) );
    }

    // TODO Store all the fields of type surfaceVectorField
}

preciceAdapter::Adapter::~Adapter()
{

    // Delete the copied fields for checkpointing
    if ( checkpointingEnabled_ )
    {
        adapterInfo( "Deleting the checkpoints... ", "debug" );
        for ( uint i = 0; i < volScalarFieldCopies_.size(); i++ )
        {
            delete volScalarFieldCopies_.at( i );
        }
        volScalarFieldCopies_.clear();

        for ( uint i = 0; i < volVectorFieldCopies_.size(); i++ )
        {
            delete volVectorFieldCopies_.at( i );
        }
        volVectorFieldCopies_.clear();

        for ( uint i = 0; i < surfaceScalarFieldCopies_.size(); i++ )
        {
            delete surfaceScalarFieldCopies_.at( i );
        }
        surfaceScalarFieldCopies_.clear();

        // TODO Add delete for other types (if any)
    }

    adapterInfo( "Deleting the interfaces...", "debug" );
    for ( uint i = 0; i < interfaces_.size(); i++ )
    {
        delete interfaces_.at( i );
    }
    interfaces_.clear();

    adapterInfo( "Finalizing the preCICE solver interface...", "debug" );
    precice_->finalize(); // TODO move

    adapterInfo( "Destroying the preCICE solver interface...", "debug" );
    // TODO: throws segmentation fault if it has not been initialized at premature exit.
    delete precice_;
}
