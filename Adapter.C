// TODO Ensure that the namespaces are full, i.e. that `Foam::` is used everywhere that is appropriate, for easier understanding. Or just drop it.

#include "Adapter.H"
#include "Interface.H"

#include "IOstreams.H"

#include "CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "CouplingDataUser/CouplingDataReader/HeatFluxBoundaryCondition.h"
#include "CouplingDataUser/CouplingDataReader/BuoyantPimpleHeatFluxBoundaryCondition.h"

#include "CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"
#include "CouplingDataUser/CouplingDataWriter/HeatFluxBoundaryValues.h"
#include "CouplingDataUser/CouplingDataWriter/BuoyantPimpleHeatFluxBoundaryValues.h"


preciceAdapter::Adapter::Adapter(const Foam::Time& runTime, const Foam::fvMesh& mesh)
:
runTime_(runTime),
mesh_(mesh)
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

    // Get the solver name
    runTime_.controlDict().readIfPresent("application", applicationName_);
    Foam::Info << "---[preciceAdapter] Application: " << applicationName_ << Foam::nl;

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
        Foam::Info << "---[preciceAdapter] [TODO] Add checkpoint fields (decide which)." << Foam::nl;
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

    // Get the thermophysical model
    Foam::Info << "---[preciceAdapter] Specifying thermoModel..." << Foam::nl;
    if (mesh_.foundObject<Foam::basicThermo>("thermophysicalProperties")) {
        Foam::Info << "---[preciceAdapter]   - Found 'thermophysicalProperties', refering to 'basicThermo'." << Foam::nl;
        thermo_ = const_cast<Foam::basicThermo*>(&mesh_.lookupObject<Foam::basicThermo>("thermophysicalProperties"));
    } else {
        Foam::Info << "---[preciceAdapter]   - Did not find 'thermophysicalProperties', no thermoModel specified." << Foam::nl;
        if (mesh_.foundObject<Foam::volScalarField>("T")) {
            Foam::Info << "---[preciceAdapter]   - Found T, however." << Foam::nl;
        }
    }

    // Get the turbulence model
    Foam::Info << "---[preciceAdapter] Specifying turbulenceModel...." << Foam::nl;
    if (mesh_.foundObject<Foam::compressible::turbulenceModel>("turbulenceProperties")) {
        Foam::Info << "---[preciceAdapter]   - Found 'turbulenceProperties', refering to 'compressible::turbulenceModel'." << Foam::nl;
        turbulence_ = const_cast<Foam::compressible::turbulenceModel*>(&mesh_.lookupObject<Foam::compressible::turbulenceModel>("turbulenceProperties"));
    } else {
        Foam::Info << "---[preciceAdapter]   - Did not find 'turbulenceProperties', no turbulenceModel specified." << Foam::nl;
    }

    // Create interfaces
    Foam::Info << "---[preciceAdapter] Creating interfaces..." << Foam::nl;
    for ( uint i = 0; i < config_.interfaces().size(); i++ )
    {
        Foam::Info << "---[preciceAdapter]   new interface" << Foam::nl;
        Interface * interface = new Interface( *precice_, mesh_, config_.interfaces().at( i ).meshName, config_.interfaces().at( i ).patchNames );
        Foam::Info << "---[preciceAdapter]   push back" << Foam::nl;
        interfaces_.push_back( interface );
        Foam::Info << "---[preciceAdapter] Interface created on mesh "
                   << config_.interfaces().at( i ).meshName
                   << Foam::nl;

        // TODO: Add coupling data users for 'sink temperature' and for 'heat transfer coefficient'
        Foam::Info << "---[preciceAdapter] Add coupling data writers" << Foam::nl;
        for ( uint j = 0; j < config_.interfaces().at( i ).writeData.size(); j++ )
        {
            std::string dataName = config_.interfaces().at( i ).writeData.at( j );

            if ( dataName.compare( "Temperature" ) == 0 )
            {
                if (thermo_) {
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( &thermo_->T() );
                    interface->addCouplingDataWriter( dataName, bw );
                    Foam::Info << "---[preciceAdapter]   Added Temperature from thermoModel." << Foam::nl;
                } else {
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) ); // TODO Mesh does not have T()
                    interface->addCouplingDataWriter( dataName, bw );
                    Foam::Info << "---[preciceAdapter]   Added Temperature from T." << Foam::nl;
                }
            }

            if ( dataName.compare( "Heat-Flux" ) == 0 )
            {
            	if ( applicationName_.compare( "buoyantPimpleFoam" ) == 0 )
            	{
                    if ( thermo_ && turbulence_ ) {
                        BuoyantPimpleHeatFluxBoundaryValues * bw = new BuoyantPimpleHeatFluxBoundaryValues( &thermo_->T(), thermo_, turbulence_ );
                        interface->addCouplingDataWriter( dataName, bw );
                        Foam::Info << "---[preciceAdapter]    Added Heat Flux for buoyantPimpleFoam." << Foam::nl;
                    } else {
                        Foam::Info << "---[preciceAdapter]   Problem: no thermo or no turbulence model specified." << Foam::nl;
                    }
            	}
            	else
            	{
                    if ( thermo_ ) {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( &thermo_->T(), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        Foam::Info << "---[preciceAdapter]    Added Heat Flux with temperature from thermoModel." << Foam::nl;
                    } else {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        Foam::Info << "---[preciceAdapter]    Added Heat Flux with temperature from T." << Foam::nl;
                    }
            	}
            }

        }

        Foam::Info << "---[preciceAdapter] Add coupling data readers" << Foam::nl;
        for ( uint j = 0; j < config_.interfaces().at( i ).readData.size(); j++ )
        {
            std::string dataName = config_.interfaces().at( i ).readData.at( j );

            if ( dataName.compare( "Temperature" ) == 0 )
            {
                if ( thermo_ ) {
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( &thermo_->T() );
                    interface->addCouplingDataReader( dataName, br );
                    Foam::Info << "---[preciceAdapter]   Added Temperature from thermoModel." << Foam::nl;
                } else {
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) );
                    interface->addCouplingDataReader( dataName, br );
                    Foam::Info << "---[preciceAdapter]   Added Temperature from T." << Foam::nl;
                }
            }

            if ( dataName.compare( "Heat-Flux" ) == 0 )
            {
            	if ( applicationName_.compare( "buoyantPimpleFoam" ) == 0 )
            	{
                    if ( thermo_ && turbulence_ ) {
                        BuoyantPimpleHeatFluxBoundaryCondition * br = new BuoyantPimpleHeatFluxBoundaryCondition( &thermo_->T(), thermo_, turbulence_ );
                        interface->addCouplingDataReader( dataName, br );
                        Foam::Info << "---[preciceAdapter]    Added Heat Flux for buoyantPimpleFoam." << Foam::nl;
                    } else {
                        Foam::Info << "---[preciceAdapter]   Problem: no thermo or no turbulence model specified." << Foam::nl;
                    }
            	}
            	else
            	{
                    if ( thermo_ ) {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( &thermo_->T(), k);
                        interface->addCouplingDataReader( dataName, br );
                        Foam::Info << "---[preciceAdapter]    Added Heat Flux with temperature from thermoModel." << Foam::nl;
                    } else {
                        double k = 100; // TODO: IMPORTANT specify k properly (conductivity for solids-laplacianFoam_preCICE)
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataReader( dataName, br );
                        Foam::Info << "---[preciceAdapter]    Added Heat Flux with temperature from T." << Foam::nl;
                    }
            	}
            }
        }
    }

    Foam::Info << "---[preciceAdapter] [TODO] Write coupling data (for the first iteration)" << Foam::nl;
    Foam::Info << "---[preciceAdapter] [TODO] Initialize preCICE data." << Foam::nl;
    Foam::Info << "---[preciceAdapter] ---" << Foam::nl;

    Foam::Info << "---[preciceAdapter] [TODO] Read coupling data (for the first iteration)" << Foam::nl;

    // Write checkpoint (for the first iteration)
    if (checkpointingEnabled_) {
        Foam::Info << "---[preciceAdapter] [TODO] Write checkpoint (for the first iteration)" << Foam::nl;
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        Foam::Info << "---[preciceAdapter] [TODO] Adjust the solver's timestep (if fixed timestep, for the first iteration)" << Foam::nl;
    }

    return true;
}

void preciceAdapter::Adapter::execute()
{
    Foam::Info << "---[preciceAdapter] if (coupling ongoing) {" << Foam::nl;
    Foam::Info << "---[preciceAdapter]   [TODO] Write coupling data (from the previous iteration)." << Foam::nl;
    Foam::Info << "---[preciceAdapter]   [TODO] Advance preCICE (from the previous iteration)." << Foam::nl;
    Foam::Info << "---[preciceAdapter]   [TODO] Read coupling data (from the previous iteration)." << Foam::nl;

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_) {
        Foam::Info << "---[preciceAdapter]   [TODO] Adjust the solver's timestep (if fixed timestep, from the previous iteration)." << Foam::nl;
    }

    // Read checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Foam::Info << "---[preciceAdapter]   [TODO] Read checkpoint (from the previous iteration)." << Foam::nl;
    }

    // Write checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Foam::Info << "---[preciceAdapter]   [TODO] Write checkpoint (from the previous iteration)." << Foam::nl;
    }

    Foam::Info << "---[preciceAdapter]   [TODO] Write if coupling timestep complete (?)." << Foam::nl;
    Foam::Info << "---[preciceAdapter] } else {" << Foam::nl;
    Foam::Info << "---[preciceAdapter]   [TODO] Exit the loop." << Foam::nl;
    Foam::Info << "---[preciceAdapter] }" << Foam::nl;

    return;
}

void preciceAdapter::Adapter::adjustTimeStep()
{
    Foam::Info << "---[preciceAdapter] [TODO] Adjust the solver's timestep (only if dynamic timestep is used)." << Foam::nl;
    return;
}

preciceAdapter::Adapter::~Adapter()
{
    Foam::Info << "---[preciceAdapter] [TODO] Destroy the preCICE Solver Interface." << Foam::nl;

    // TODO: throws segmentation fault if it has not been initialized at premature exit.
    // delete precice_;
}
