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

using namespace Foam;

preciceAdapter::Adapter::Adapter(const Foam::Time& runTime, const Foam::fvMesh& mesh)
:
runTime_(runTime),
mesh_(mesh)
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
        runTime_.stopAt(Foam::Time::saNoWriteNow);
        FatalErrorIn("configFileCheck()")
            << "Error in the preCICE adapter:"
            << nl
            << "There was a problem reading the adapter's configuration file. "
            << "See the log for details."
            << nl
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
    if (mesh_.foundObject<Foam::basicThermo>("thermophysicalProperties")) {
        Info << "---[preciceAdapter]   - Found 'thermophysicalProperties', refering to 'basicThermo'." << nl;
        thermo_ = const_cast<Foam::basicThermo*>(&mesh_.lookupObject<Foam::basicThermo>("thermophysicalProperties"));
    } else {
        Info << "---[preciceAdapter]   - Did not find 'thermophysicalProperties', no thermoModel specified." << nl;
        if (mesh_.foundObject<Foam::volScalarField>("T")) {
            Info << "---[preciceAdapter]   - Found T, however." << nl;
        }
    }

    // Get the turbulence model
    Info << "---[preciceAdapter] Specifying turbulenceModel...." << nl;
    if (mesh_.foundObject<Foam::compressible::turbulenceModel>("turbulenceProperties")) {
        Info << "---[preciceAdapter]   - Found 'turbulenceProperties', refering to 'compressible::turbulenceModel'." << nl;
        turbulence_ = const_cast<Foam::compressible::turbulenceModel*>(&mesh_.lookupObject<Foam::compressible::turbulenceModel>("turbulenceProperties"));
    } else {
        Info << "---[preciceAdapter]   - Did not find 'turbulenceProperties', no turbulenceModel specified." << nl;
    }

    // Create interfaces
    Info << "---[preciceAdapter] Creating interfaces..." << nl;
    for ( uint i = 0; i < config_.interfaces().size(); i++ )
    {
        Info << "---[preciceAdapter]   new interface" << nl;
        Interface * interface = new Interface( *precice_, mesh_, config_.interfaces().at( i ).meshName, config_.interfaces().at( i ).patchNames );
        Info << "---[preciceAdapter]   push back" << nl;
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
                    TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) ); // TODO Mesh does not have T()
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
                        HeatFluxBoundaryValues * bw = new HeatFluxBoundaryValues( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataWriter( dataName, bw );
                        Info << "---[preciceAdapter]    Added Heat Flux with temperature from T." << nl;
                    }
            	}
            }

        }

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
                    TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")) );
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
                        HeatFluxBoundaryCondition * br = new HeatFluxBoundaryCondition( const_cast<Foam::volScalarField*>(&mesh_.lookupObject<volScalarField>("T")), k);
                        interface->addCouplingDataReader( dataName, br );
                        Info << "---[preciceAdapter]    Added Heat Flux with temperature from T." << nl;
                    }
            	}
            }
        }
    }

    Info << "---[preciceAdapter] [TODO] Write coupling data (for the first iteration)" << nl;
    Info << "---[preciceAdapter] [TODO] Initialize preCICE data." << nl;
    Info << "---[preciceAdapter] ---" << nl;

    Info << "---[preciceAdapter] [TODO] Read coupling data (for the first iteration)" << nl;

    // Write checkpoint (for the first iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter] [TODO] Write checkpoint (for the first iteration)" << nl;
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        Info << "---[preciceAdapter] [TODO] Adjust the solver's timestep (if fixed timestep, for the first iteration)" << nl;
    }

    return true;
}

void preciceAdapter::Adapter::execute()
{
    Info << "---[preciceAdapter] if (coupling ongoing) {" << nl;
    Info << "---[preciceAdapter]   [TODO] Write coupling data (from the previous iteration)." << nl;
    Info << "---[preciceAdapter]   [TODO] Advance preCICE (from the previous iteration)." << nl;
    Info << "---[preciceAdapter]   [TODO] Read coupling data (from the previous iteration)." << nl;

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_) {
        Info << "---[preciceAdapter]   [TODO] Adjust the solver's timestep (if fixed timestep, from the previous iteration)." << nl;
    }

    // Read checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter]   [TODO] Read checkpoint (from the previous iteration)." << nl;
    }

    // Write checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter]   [TODO] Write checkpoint (from the previous iteration)." << nl;
    }

    Info << "---[preciceAdapter]   [TODO] Write if coupling timestep complete (?)." << nl;
    Info << "---[preciceAdapter] } else {" << nl;
    Info << "---[preciceAdapter]   [TODO] Exit the loop." << nl;
    Info << "---[preciceAdapter] }" << nl;

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
