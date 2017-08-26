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

preciceAdapter::Adapter::Adapter(const Time& runTime, const fvMesh& mesh)
:
runTime_(runTime),
mesh_(mesh),
timestepSolver_(-1)
{
    Info << "Entered the Adapter() constructor." << nl;
    return;
}

bool preciceAdapter::Adapter::configure()
{
    Info << "Entered Adapter::configure()." << nl;

    config_ = preciceAdapter::Config();

    // Read the adapter's configuration file
    if ( !config_.configFileRead() )
    {
        runTime_.stopAt(Time::saNoWriteNow);
        FatalErrorIn("configFileCheck()")
            << "Error in the preCICE adapter:"
            << nl
            << "There was a problem reading the adapter's configuration file. "
            << "See the log for details."
            << nl
            << exit(FatalError);
        return false;
    }

    // Copy the variables from the config_
    participantName_ = config_.participantName();
    preciceConfigFilename_ = config_.preciceConfigFilename();
    subcyclingAllowed_ = config_.subcyclingAllowed();
    checkpointingEnabled_ = config_.checkpointingEnabled();

    // Get the solver name
    runTime_.controlDict().readIfPresent("application", applicationName_);
    Info << "---[preciceAdapter] Application: " << applicationName_ << nl;

    // Check the timestep type (fixed vs adjustable)
    Info << "---[preciceAdapter] Check the timestep type (fixed vs adjustable)." << nl;
    adjustableTimestep_ = runTime_.controlDict().lookupOrDefault("adjustTimeStep", false);

    if ( adjustableTimestep_ ) {
        Info << "---[preciceAdapter]   Timestep type: adjustable." << nl;
    } else {
        Info << "---[preciceAdapter]   Timestep type: fixed." << nl;
    }

    // Add fields in the checkpointing list
    if ( checkpointingEnabled_ ) {
        Info << "---[preciceAdapter] [TODO] Add checkpoint fields (decide which)." << nl;
    }

    // Initialize preCICE
    Info << "---[preciceAdapter] Create the preCICE solver interface." << nl;
    int MPIEnabled = 0;
    int MPIRank = 0;
    int MPISize = 1;

    MPI_Initialized( &MPIEnabled );
    Info << "---[preciceAdapter]   MPI used: " << MPIEnabled << nl;

    if ( MPIEnabled )
    {
        MPI_Comm_rank( MPI_COMM_WORLD, &MPIRank );
        Info << "---[preciceAdapter]   MPI rank: " << MPIRank << nl;

        MPI_Comm_size( MPI_COMM_WORLD, &MPISize );
        Info << "---[preciceAdapter]   MPI size: " << MPISize << nl;
    }

    precice_ = new precice::SolverInterface( participantName_, MPIRank, MPISize );
    Info << "---[preciceAdapter]   preCICE solver interface was created." << nl;

    Info << "---[preciceAdapter] Configure preCICE." << nl;
    precice_->configure( preciceConfigFilename_ );

    // Get the thermophysical model
    Info << "---[preciceAdapter] Specifying thermoModel..." << nl;
    if (mesh_.foundObject<basicThermo>("thermophysicalProperties")) {
        Info << "---[preciceAdapter]   - Found 'thermophysicalProperties', refering to 'basicThermo'." << nl;
        thermo_ = const_cast<basicThermo*>(&mesh_.lookupObject<basicThermo>("thermophysicalProperties"));
    } else {
        Info << "---[preciceAdapter]   - Did not find 'thermophysicalProperties', no thermoModel specified." << nl;
        if (mesh_.foundObject<volScalarField>("T")) {
            Info << "---[preciceAdapter]   - Found T, however." << nl;
        }
    }

    // Get the turbulence model
    Info << "---[preciceAdapter] Specifying turbulenceModel...." << nl;
    if (mesh_.foundObject<compressible::turbulenceModel>("turbulenceProperties")) {
        Info << "---[preciceAdapter]   - Found 'turbulenceProperties', refering to 'compressible::turbulenceModel'." << nl;
        turbulence_ = const_cast<compressible::turbulenceModel*>(&mesh_.lookupObject<compressible::turbulenceModel>("turbulenceProperties"));
    } else {
        Info << "---[preciceAdapter]   - Did not find 'turbulenceProperties', no turbulenceModel specified." << nl;
    }

    // Create interfaces
    Info << "---[preciceAdapter] Creating interfaces..." << nl;
    for ( uint i = 0; i < config_.interfaces().size(); i++ )
    {
        Interface * interface = new Interface( *precice_, mesh_, config_.interfaces().at( i ).meshName, config_.interfaces().at( i ).patchNames );
        interfaces_.push_back( interface );
        Info << "---[preciceAdapter] Interface created on mesh "
             << config_.interfaces().at( i ).meshName
             << nl;

        // TODO: Add coupling data users for 'sink temperature' and for 'heat transfer coefficient'
        Info << "---[preciceAdapter] Add coupling data writers" << nl;
        for ( uint j = 0; j < config_.interfaces().at( i ).writeData.size(); j++ )
        {
            std::string dataName = config_.interfaces().at( i ).writeData.at( j );

            if ( dataName.compare( "Temperature" ) == 0 )
            {
                if (thermo_) {
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( &thermo_->T() );
                    interface->addCouplingDataWriter( dataName, bw );
                    Info << "---[preciceAdapter]   Added Temperature from thermoModel." << nl;
                } else {
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) ); // TODO Mesh does not have T()
                    interface->addCouplingDataWriter( dataName, bw );
                    Info << "---[preciceAdapter]   Added Temperature from T." << nl;
                }
            }

            if ( dataName.compare( "Heat-Flux" ) == 0 )
            {
                if ( applicationName_.compare( "buoyantPimpleFoam" ) == 0 )
                {
                    if ( thermo_ && turbulence_ ) {
                        BuoyantPimpleHeatFluxBoundaryValues * bw = new BuoyantPimpleHeatFluxBoundaryValues( &thermo_->T(), thermo_, turbulence_ );
                        interface->addCouplingDataWriter( dataName, bw );
                        Info << "---[preciceAdapter]    Added Heat Flux for buoyantPimpleFoam." << nl;
                    } else {
                        Info << "---[preciceAdapter]   Problem: no thermo or no turbulence model specified." << nl;
                    }
                }
                else
                {
                    if ( thermo_ ) {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( &thermo_->T(), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        Info << "---[preciceAdapter]    Added Heat Flux with temperature from thermoModel." << nl;
                    } else {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        Info << "---[preciceAdapter]    Added Heat Flux with temperature from T." << nl;
                    }
                }
            }

            if ( dataName.compare( "Heat-Transfer-Coefficient" ) == 0 )
            {
                // TODO Implement Heat Transfer Coefficient
                Info << "---[preciceAdapter]    [TODO] Heat Transfer Coefficient not implemented yet." << nl;
            }

            if ( dataName.compare( "Sink-Temperature" ) == 0 )
            {
                // TODO Implement Sink Temperature
                Info << "---[preciceAdapter]    [TODO] Sink Temperature not implemented yet." << nl;
            }

        } // end add coupling data writers

        Info << "---[preciceAdapter] Add coupling data readers" << nl;
        for ( uint j = 0; j < config_.interfaces().at( i ).readData.size(); j++ )
        {
            std::string dataName = config_.interfaces().at( i ).readData.at( j );

            if ( dataName.compare( "Temperature" ) == 0 )
            {
                if ( thermo_ ) {
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( &thermo_->T() );
                    interface->addCouplingDataReader( dataName, br );
                    Info << "---[preciceAdapter]   Added Temperature from thermoModel." << nl;
                } else {
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) );
                    interface->addCouplingDataReader( dataName, br );
                    Info << "---[preciceAdapter]   Added Temperature from T." << nl;
                }
            }

            if ( dataName.compare( "Heat-Flux" ) == 0 )
            {
            	if ( applicationName_.compare( "buoyantPimpleFoam" ) == 0 )
            	{
                    if ( thermo_ && turbulence_ ) {
                        BuoyantPimpleHeatFluxBoundaryCondition * br = new BuoyantPimpleHeatFluxBoundaryCondition( &thermo_->T(), thermo_, turbulence_ );
                        interface->addCouplingDataReader( dataName, br );
                        Info << "---[preciceAdapter]    Added Heat Flux for buoyantPimpleFoam." << nl;
                    } else {
                        Info << "---[preciceAdapter]   Problem: no thermo or no turbulence model specified." << nl;
                    }
                }
                else
                {
                    if ( thermo_ ) {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( &thermo_->T(), k);
                        interface->addCouplingDataReader( dataName, br );
                        Info << "---[preciceAdapter]    Added Heat Flux with temperature from thermoModel." << nl;
                    } else {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( const_cast<volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataReader( dataName, br );
                        Info << "---[preciceAdapter]    Added Heat Flux with temperature from T." << nl;
                    }
            	}
            }

            if ( dataName.compare( "Heat-Transfer-Coefficient" ) == 0 )
            {
                // TODO Implement Heat Transfer Coefficient
                Info << "---[preciceAdapter]    [TODO] Heat Transfer Coefficient not implemented yet." << nl;
            }

            if ( dataName.compare( "Sink-Temperature" ) == 0 )
            {
                // TODO Implement Sink Temperature
                Info << "---[preciceAdapter]    [TODO] Sink Temperature not implemented yet." << nl;
            }

        } // end add coupling data readers
    }

    Info << "---[preciceAdapter] [In Progress] Write coupling data (for the first iteration) & initialize preCICE data." << nl;
    this->initialize();

    Info << "---[preciceAdapter] ---" << nl;

    Info << "---[preciceAdapter] [In Progress] Read coupling data (for the first iteration)" << nl;
    this->readCouplingData();

    // Write checkpoint (for the first iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter] [TODO] Write checkpoint (for the first iteration)" << nl;
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        Info << "---[preciceAdapter] [In Progress] Adjust the solver's timestep (if fixed timestep, for the first iteration)" << nl;
        this->adjustSolverTimeStep();
    }

    return true;
}

void preciceAdapter::Adapter::execute()
{
    Info << "---[preciceAdapter] if (coupling ongoing) {" << nl;
    if ( this->isCouplingOngoing() ) {
        Info << "---[preciceAdapter]   [In Progress] Write coupling data (from the previous iteration)." << nl;
        this->writeCouplingData();

        Info << "---[preciceAdapter]   [In Progress] Advance preCICE (from the previous iteration)." << nl;
        this->advance();

        Info << "---[preciceAdapter]   [In Progress] Read coupling data (from the previous iteration)." << nl;
        this->readCouplingData();

        // Adjust the timestep, if it is fixed
        if (!adjustableTimestep_) {
            Info << "---[preciceAdapter]   [In Progress] Adjust the solver's timestep (if fixed timestep, from the previous iteration)." << nl;
            this->adjustSolverTimeStep();
        }

        // Read checkpoint (from the previous iteration)
        if (checkpointingEnabled_) {
            Info << "---[preciceAdapter]   [TODO] Read checkpoint (from the previous iteration)." << nl;
        }

        // Write checkpoint (from the previous iteration)
        if (checkpointingEnabled_) {
            Info << "---[preciceAdapter]   [TODO] Write checkpoint (from the previous iteration)." << nl;
        }

        Info << "---[preciceAdapter]   [In Progress] Write if coupling timestep complete (?)." << nl;
        if (this->isCouplingTimestepComplete()) {
            runTime_.write();
        }
        else
        {
            Info << "---[preciceAdapter]   [TODO] Exit the loop." << nl;
        }
    }

    return;
}

void preciceAdapter::Adapter::adjustTimeStep()
{
    Info << "---[preciceAdapter] [TODO] Adjust the solver's timestep (only if dynamic timestep is used)." << nl;
    return;
}

preciceAdapter::Adapter::~Adapter()
{
    Info << "---[preciceAdapter] [TODO] Destroy the preCICE Solver Interface." << nl;

    // TODO: throws segmentation fault if it has not been initialized at premature exit.
    // delete precice_;
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
            Info << "The solver's timestep cannot be smaller than the "
                 << "coupling timestep, because subcycling has not been activated. "
                 << "Forcing the solver to use the coupling timestep."
                 << nl;
                 timestepSolver_ = timestepPrecice_;
        }
        else
        {
            Info << "The solver's timestep is smaller than the "
                 << "coupling timestep. Subcycling..."
                 << nl;
            timestepSolver_ = timestepSolverDetermined;
        }
    }
    else if ( timestepSolverDetermined > timestepPrecice_ )
    {
        Info << "The solver's timestep cannot be larger than the coupling timestep. "
             << "Adjusting from "
             << timestepSolverDetermined
             << "to"
             << timestepPrecice_
             << nl;
        timestepSolver_ = timestepPrecice_;
    }
    else
    {
        timestepSolver_ = timestepPrecice_;
    }

    // Update the solver's timestep
    const_cast<Time&>(runTime_).setDeltaT( timestepSolver_ );
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
