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

#define INFO_STR_ADAPTER "---[preciceAdapter] "
#define INFO_STR_DEBUG "[DEBUG] "

preciceAdapter::Adapter::Adapter(const Time& runTime, const fvMesh& mesh)
:
runTime_(runTime),
mesh_(mesh)
{
    adapterInfo( "The preciceAdapter was loaded.", "info" );
    return;
}

void preciceAdapter::Adapter::adapterInfo(const std::string message, const std::string level = "info")
{
    if ( level.compare("info") == 0 )
    {
        // Prepend the message with a string
        Info << INFO_STR_ADAPTER
             << message.c_str()
             << nl;
    }
    else if ( level.compare("warning") == 0 )
    {
        // Produce a warning message with cyan header
        WarningInFunction
             << "\033[36m" // cyan color
             << "Warning in the preCICE adapter: "
             << "\033[0m" // restore color
             << nl
             << message.c_str()
             << nl;
    }
    else if ( level.compare("error") == 0 )
    {
        // Produce an error message with red header
        // and exit the functionObject.
        // It will also exit the simulation, unless it
        // is called inside the functionObject's read().
        FatalErrorInFunction
             << "\033[31m" // red color
             << "Error in the preCICE adapter: "
             << "\033[0m" // restore color
             << nl
             << message.c_str()
             << exit(FatalError);
    }
    else if ( level.compare("debug") == 0 )
    {
        if ( ADAPTER_DEBUG_MODE )
        {
            Info << INFO_STR_ADAPTER
                 << INFO_STR_DEBUG
                 << message.c_str()
                 << nl;
        }
    }
    else if ( level.compare("dev") == 0 )
    {
        Info << "\033[35m" // cyan color
             << INFO_STR_ADAPTER
             << "[under development] "
             << "\033[0m " // restore color
             << message.c_str()
             << nl;
    }
    else
    {
        Info << INFO_STR_ADAPTER
             << "[unknown info level] "
             << message.c_str()
             << nl;
    }

    return;
}

bool preciceAdapter::Adapter::configFileCheck(const std::string adapterConfigFileName)
{
    adapterInfo( "Checking the adapter's YAML configuration file...", "debug" );

    bool configErrors = false;

    YAML::Node adapterConfig = YAML::LoadFile(adapterConfigFileName);

    // TODO Consider simplifying
    // Check if the "participant" node exists
    if ( !adapterConfig["participant"] )
    {
        adapterInfo( "The 'participant' node is missing in " + adapterConfigFileName + ".", "warning" );
        configErrors = true;
    }

    // Check if the "precice-config-file" node exists
    if ( !adapterConfig["precice-config-file"] )
    {
        adapterInfo( "The 'precice-config-file' node is missing in " + adapterConfigFileName + ".", "warning" );
        configErrors = true;
        // TODO Check if the specified file exists
    }

    // Check if the "interfaces" node exists
    if ( !adapterConfig["interfaces"] )
    {
        adapterInfo( "The 'interfaces' node is missing in " + adapterConfigFileName + ".", "warning" );
        configErrors = true;
    } else {
        for ( uint i = 0; i < adapterConfig["interfaces"].size(); i++ )
        {
            if ( !adapterConfig["interfaces"][i]["mesh"] )
            {
                adapterInfo( "The 'mesh' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning" );
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["patches"] )
            {
                adapterInfo( "The 'patches' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning" );
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["write-data"] )
            {
                adapterInfo( "The 'write-data' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning" );
                configErrors = true;
                // TODO Add check for allowed values.
            }
            if ( !adapterConfig["interfaces"][i]["read-data"] )
            {
                adapterInfo( "The 'read-data' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning" );
                configErrors = true;
                // TODO Add check for allowed values.
            }
        }
    }

    if ( !configErrors )
    {
        adapterInfo( "The adapter's YAML configuration file " + adapterConfigFileName + " is complete." , "debug");
    }

    return !configErrors;
}


bool preciceAdapter::Adapter::configFileRead()
{

    // Check the configuration file
    const std::string adapterConfigFileName = runTime_.path() + "/precice-adapter-config.yml";
    adapterInfo( "Reading the adapter's YAML configuration file " + adapterConfigFileName + "...", "debug" );

    if ( !configFileCheck(adapterConfigFileName) ) return false;

    // Load the YAML file
    YAML::Node adapterConfig_ = YAML::LoadFile(adapterConfigFileName);

    // Read the preCICE participant name
    participantName_ = adapterConfig_["participant"].as<std::string>();
    adapterInfo( "  participant : " + participantName_, "debug" );

    // Read the preCICE configuration file name
    preciceConfigFilename_ = adapterConfig_["precice-config-file"].as<std::string>();
    adapterInfo( "  precice-config-file : " + preciceConfigFilename_, "debug" );

    // TODO Read the coupling interfaces configuration
    YAML::Node adapterConfigInterfaces = adapterConfig_["interfaces"];
    adapterInfo( "  interfaces : ", "debug" );
    for (uint i = 0; i < adapterConfigInterfaces.size(); i++)
    {
        struct InterfaceConfig interfaceConfig;
        interfaceConfig.meshName = adapterConfigInterfaces[i]["mesh"].as<std::string>();
        adapterInfo( "  - mesh      : " + interfaceConfig.meshName, "debug" );

        adapterInfo( "    patches   : ", "debug");
        for ( uint j = 0; j < adapterConfigInterfaces[i]["patches"].size(); j++)
        {
            interfaceConfig.patchNames.push_back( adapterConfigInterfaces[i]["patches"][j].as<std::string>() );
            adapterInfo( "      " + adapterConfigInterfaces[i]["patches"][j].as<std::string>(), "debug" );
        }

        // TODO: Consider simplification
        if ( adapterConfigInterfaces[i]["write-data"] )
        {
            adapterInfo( "    write-data : ", "debug" );
            if ( adapterConfigInterfaces[i]["write-data"].size() > 0 )
            {
                // TODO Check: before it was adapterConfigInterfaces[i]["read-data"].size()
                for ( uint j = 0; j < adapterConfigInterfaces[i]["write-data"].size(); j++)
                {
                    interfaceConfig.writeData.push_back( adapterConfigInterfaces[i]["write-data"][j].as<std::string>() );
                    adapterInfo( "      " + adapterConfigInterfaces[i]["write-data"][j].as<std::string>(), "debug" );
                }
            }
            else
            {
                interfaceConfig.writeData.push_back( adapterConfigInterfaces[i]["write-data"].as<std::string>() );
                adapterInfo( "      " + adapterConfigInterfaces[i]["write-data"].as<std::string>(), "debug" );
            }
        }

        // TODO: Consider simplification
        if ( adapterConfigInterfaces[i]["read-data"] )
        {
            adapterInfo( "    read-data : ", "debug" );
            if ( adapterConfigInterfaces[i]["read-data"].size() > 0 )
            {
                for ( uint j = 0; j < adapterConfigInterfaces[i]["read-data"].size(); j++)
                {
                    interfaceConfig.readData.push_back( adapterConfigInterfaces[i]["read-data"][j].as<std::string>() );
                    adapterInfo( "      " + adapterConfigInterfaces[i]["read-data"][j].as<std::string>(), "debug" );
                }
            }
            else
            {
                interfaceConfig.readData.push_back( adapterConfigInterfaces[i]["read-data"].as<std::string>() );
                adapterInfo( "      " + adapterConfigInterfaces[i]["read-data"].as<std::string>(), "debug" );
            }
        }

        interfacesConfig_.push_back( interfaceConfig );
    }

    // Set the subcyclingAllowed_ switch
    if ( adapterConfig_["subcycling"] )
    {
        subcyclingAllowed_ = adapterConfig_["subcycling"].as<bool>();
    }
    adapterInfo( "    subcycling : " + std::to_string(subcyclingAllowed_), "debug" );

    // Set the preventEarlyExit_ switch
    if ( adapterConfig_["preventEarlyExit"] )
    {
        preventEarlyExit_ = adapterConfig_["preventEarlyExit"].as<bool>();
    }
    adapterInfo( "    prevent early exit : " + std::to_string(preventEarlyExit_), "debug" );

    return true;
}


void preciceAdapter::Adapter::configure()
{

    // Read the adapter's configuration file
    if ( !configFileRead() )
    {
        // This method is called from the functionObject's read() method,
        // which is called by the Foam::functionObjectList::read() method.
        // All the exceptions triggered in this method are caught as
        // warnings and the simulation continues simply without the
        // functionObject. However, we want the simulation to exit with an
        // error in case something is wrong. We store the information that
        // there was an error and it will be handled by the first call to
        // the functionObject's execute(), which can throw errors normally.
        errorsInConfigure = true;

        return;
    }

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

    // Initialize preCICE
    adapterInfo( "Creating the preCICE solver interface...", "debug" );
    int MPIEnabled = 0;
    int MPIRank = 0;
    int MPISize = 1;

    MPI_Initialized( &MPIEnabled );
    adapterInfo( "  MPI used: " + std::to_string(MPIEnabled), "debug" );

    if ( MPIEnabled )
    {
        MPI_Comm_rank( MPI_COMM_WORLD, &MPIRank );
        adapterInfo( "  MPI rank: " + std::to_string(MPIRank), "debug" );

        MPI_Comm_size( MPI_COMM_WORLD, &MPISize );
        adapterInfo( "  MPI size: " + std::to_string(MPISize), "debug" );
    }

    precice_ = new precice::SolverInterface( participantName_, MPIRank, MPISize );
    adapterInfo( "  preCICE solver interface was created.", "debug" );

    adapterInfo( "Configuring preCICE...", "debug" );
    precice_->configure( preciceConfigFilename_ );
    adapterInfo( "  preCICE was configured.", "debug" );

    // Get the thermophysical model
    adapterInfo( "Specifying the thermophysical model...", "debug" );
    if (mesh_.foundObject<basicThermo>("thermophysicalProperties")) {
        adapterInfo( "  Found 'thermophysicalProperties', refering to 'basicThermo'.", "debug" );
        thermo_ = const_cast<basicThermo*>(&mesh_.lookupObject<basicThermo>("thermophysicalProperties"));
    } else {
        adapterInfo( "  Did not find 'thermophysicalProperties', no thermoModel specified.", "debug" );
        if (mesh_.foundObject<volScalarField>("T")) {
            adapterInfo( "  Found the T object.", "debug" );
        }
    }

    // Get the turbulence model
    adapterInfo( "Specifying the turbulence model...", "debug" );
    if (mesh_.foundObject<compressible::turbulenceModel>("turbulenceProperties")) {
        adapterInfo( "  Found 'turbulenceProperties', refering to 'compressible::turbulenceModel'.", "debug" );
        turbulence_ = const_cast<compressible::turbulenceModel*>(&mesh_.lookupObject<compressible::turbulenceModel>("turbulenceProperties"));
    } else {
        adapterInfo( "  Did not find 'turbulenceProperties', no turbulenceModel specified.", "debug" );
    }

    // Create interfaces
    adapterInfo( "Creating interfaces...", "debug" );
    for ( uint i = 0; i < interfacesConfig_.size(); i++ )
    {
        Interface * interface = new Interface( *precice_, mesh_, interfacesConfig_.at( i ).meshName, interfacesConfig_.at( i ).patchNames );
        interfaces_.push_back( interface );
        adapterInfo( "Interface created on mesh" + interfacesConfig_.at( i ).meshName, "debug" );

        // TODO: Add coupling data users for 'sink temperature' and for 'heat transfer coefficient'
        adapterInfo( "Adding coupling data writers...", "dev" );
        for ( uint j = 0; j < interfacesConfig_.at( i ).writeData.size(); j++ )
        {
            std::string dataName = interfacesConfig_.at( i ).writeData.at( j );

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
                        adapterInfo( "Problem: no thermo or no turbulence model specified", "error" );
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
                adapterInfo( "Heat Transfer Coefficient not implemented yet!", "error" );
            }

            if ( dataName.compare( "Sink-Temperature" ) == 0 )
            {
                // TODO Implement Sink Temperature
                adapterInfo( "Sink Temperature not implemented yet!", "error" );
            }

        } // end add coupling data writers

        adapterInfo( "Adding coupling data readers...", "dev" );
        for ( uint j = 0; j < interfacesConfig_.at( i ).readData.size(); j++ )
        {
            std::string dataName = interfacesConfig_.at( i ).readData.at( j );

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
                        adapterInfo( "Problem: no thermo or no turbulence model specified", "error" );
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
                adapterInfo( "Heat Transfer Coefficient not implemented yet!", "error" );
            }

            if ( dataName.compare( "Sink-Temperature" ) == 0 )
            {
                // TODO Implement Sink Temperature
                adapterInfo( "Sink Temperature not implemented yet!", "error" );
            }

        } // end add coupling data readers
    }

    // Initialize preCICE and exchange the first coupling data
    initialize();

    // Read the received coupling data
    readCouplingData();

    // If checkpointing is required, specify the checkpointed fields
    // and write the first checkpoint
    if ( isWriteCheckpointRequired() )
    {
        checkpointing_ = true;

        // Setup the checkpointing (find and add fields to checkpoint)
        setupCheckpointing();

        // Write checkpoint (for the first iteration)
        writeCheckpoint();
        fulfilledWriteCheckpoint();
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        adjustSolverTimeStep();
    }

    // If the solver tries to end before the coupling is complete,
    // e.g. because the solver's endTime was smaller or (in implicit
    // coupling) equal with the max-time specified in preCICE,
    // problems may occur near the end of the simulation,
    // as the function object may be called only once near the end.
    // See the implementation of Foam::Time::run() for more details.
    // To prevent this, we set the solver's endTime to "infinity"
    // and let only preCICE control the end of the simulation.
    // This has the side-effect of not triggering the end() method
    // in any function object normally. Therefore, we trigger it
    // when preCICE dictates to stop the coupling.
    // However, the user can disable this behavior in the configuration.
    if ( preventEarlyExit_ )
    {
        adapterInfo( "Setting the solver's endTime to infinity to prevent early exits. "
                     "Only preCICE will control the simulation's endTime. "
                     "Any functionObject's end() method will be triggered by the adapter. "
                     "You may disable this behavior in the adapter's configuration.",
                     "warning");
        const_cast<Time&>(runTime_).setEndTime( GREAT );
    }

    return;
}

void preciceAdapter::Adapter::execute()
{
    if ( errorsInConfigure )
    {
        // Handle any errors during configure().
        // See the comments in configure() for details.
        adapterInfo( "There was a problem while configuring the adapter. "
                     "See the log for details.", "error" );
    }

    // The solver has already solved the equations for this timestep.
    // Now call the adapter's methods to perform the coupling.

    // Write the coupling data in the buffer
    writeCouplingData();

    // Advance preCICE
    advance();

    // Read checkpoint if required
    if ( isReadCheckpointRequired() )
    {
        readCheckpoint();
        fulfilledReadCheckpoint();
    }

    // Read the received coupling data from the buffer
    readCouplingData();

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_) {
        adjustSolverTimeStep();
    }

    // Write checkpoint if required
    if ( isWriteCheckpointRequired() )
    {
        writeCheckpoint();
        fulfilledWriteCheckpoint();
    }

    if (isCouplingTimestepComplete()) {
        adapterInfo( "The coupling timestep is complete.", "info" );
        adapterInfo( "  Writing the results...", "dev" );
        // TODO write() or writeNow()?
        // TODO Check if results are written at the last timestep.
        // TODO Check for implicit coupling to avoid multiple writes.
        const_cast<Time&>(runTime_).writeNow();
    }

    // If the coupling is not going to continue, tear down everything
    // and stop the simulation.
    if ( !isCouplingOngoing() )
    {
        adapterInfo( "The coupling completed.", "info" );

        // Finalize the preCICE solver interface and delete data
        finalize();

        // Tell OpenFOAM to stop the simulation.
        // Set the solver's endTime to now. The next evaluation of
        // runTime.run() will be false and the solver will exit.
        const_cast<Time&>(runTime_).setEndTime(runTime_.value());

        if ( preventEarlyExit_ )
        {
            adapterInfo( "The simulation was ended by preCICE. "
                         "Calling the end() methods of any functionObject.",
                         "info" );
            const_cast<Time&>(runTime_).functionObjects().end();
        }
    }

    return;
}

void preciceAdapter::Adapter::adjustTimeStep()
{
    adjustSolverTimeStep();

    return;
}

void preciceAdapter::Adapter::readCouplingData()
{
    adapterInfo( "Reading coupling data...", "debug" );

    for ( uint i = 0; i < interfaces_.size(); i++ )
    {
        interfaces_.at( i )->readCouplingData();
    }
}

void preciceAdapter::Adapter::writeCouplingData()
{
    adapterInfo( "Writing coupling data...", "debug" );

    for ( uint i = 0; i < interfaces_.size(); i++ )
    {
        interfaces_.at( i )->writeCouplingData();
    }
}

void preciceAdapter::Adapter::initialize()
{
    adapterInfo( "Iniializing the preCICE solver interface...", "debug" );
    timestepPrecice_ = precice_->initialize();

    preciceInitialized_ = true;

    if ( precice_->isActionRequired( precice::constants::actionWriteInitialData() ) )
    {
        writeCouplingData();
        precice_->fulfilledAction( precice::constants::actionWriteInitialData() );
    }

    adapterInfo( "Initializing preCICE data...", "debug");
    precice_->initializeData();

    adapterInfo( "preCICE was configured and initialized", "info" );
}

void preciceAdapter::Adapter::finalize()
{
    if ( NULL != precice_ && preciceInitialized_ && !isCouplingOngoing() )
    {
        adapterInfo( "Finalizing the preCICE solver interface...", "debug" );

        // Finalize the preCICE solver interface
        precice_->finalize();

        preciceInitialized_ = false;

        // Delete the solver interface and all the related data
        teardown();
    }
    else
    {
        adapterInfo( "Could not finalize preCICE.", "error" );
    }
}

void preciceAdapter::Adapter::advance()
{
    adapterInfo( "Advancing preCICE...", "info" );

    timestepPrecice_ = precice_->advance( timestepSolver_ );
}

void preciceAdapter::Adapter::adjustSolverTimeStep()
{
    adapterInfo( "Adjusting the solver's timestep...", "debug" );

    // The timestep size that the solver has determined that it wants to use
    double timestepSolverDetermined;

    /* In this method, the adapter overwrites the timestep used by OpenFOAM.
       If the timestep is not adjustable, OpenFOAM will not try to re-estimate
       the timestep or read it again from the controlDict. Therefore, store
       the value that the timestep has is the beginning and try again to use this
       in every iteration.
       // TODO Treat also the case where the user modifies the timestep
       // in the controlDict during the simulation.
    */

    // Is the timestep adjustable or fixed?
    if ( !adjustableTimestep_ )
    {
        // Have we already stored the timestep?
        if ( !useStoredTimestep_ )
        {
            // Show a warning if runTimeModifiable is set
            if ( runTime_.runTimeModifiable() )
            {
                adapterInfo("You have enabled 'runTimeModifiable' in the "
                            "controlDict. The preciceAdapter does not yet "
                            "fully support this functionality when "
                            "'adjustableTimestep' is not enabled. "
                            "If you modify the 'deltaT' in the controlDict "
                            "during the simulation, it will not be updated.",
                            "warning");
            }

            // Store the value
            timestepStored_ = runTime_.deltaT().value();

            // Ok, we stored it once, we will use this from now on
            useStoredTimestep_ = true;
        }

        // Use the stored timestep as the determined solver's timestep
        timestepSolverDetermined = timestepStored_;
    }
    else
    {
        // The timestep is adjustable, so OpenFOAM will modify it
        // and therefore we can use the updated value
        timestepSolverDetermined = runTime_.deltaT().value();
    }

    /* If the solver tries to use a timestep smaller than the one determined
       by preCICE, that means that the solver is trying to subcycle.
       This may not be allowed by the user.
       If the solver tries to use a bigger timestep, then it needs to use
       the same timestep as the one determined by preCICE.
    */

    if ( timestepSolverDetermined < timestepPrecice_ )
    {
        if ( !subcyclingAllowed_ )
        {
            adapterInfo(
                "The solver's timestep cannot be smaller than the "
                "coupling timestep, because subcycling is disabled. ",
                "error"
            );
        }
        else
        {
            adapterInfo(
                "The solver's timestep is smaller than the "
                "coupling timestep. Subcycling...",
                "info"
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
        adapterInfo(
            "The solver's timestep is the same as the coupling timestep.",
            "debug"
        );
        timestepSolver_ = timestepPrecice_;
    }

    // Update the solver's timestep (but don't trigger the adjustDeltaT(),
    // which also triggers the functionObject's adjustTimeStep()) (TODO)
    const_cast<Time&>(runTime_).setDeltaT( timestepSolver_, false );
}

bool preciceAdapter::Adapter::isCouplingOngoing()
{
    bool isCouplingOngoing = false;

    // If the coupling ends before the solver ends,
    // the solver would try to access this method again,
    // giving a segmentation fault if precice_
    // was not available.
    if ( NULL != precice_ )
    {
        isCouplingOngoing = precice_->isCouplingOngoing();
    }

    return isCouplingOngoing;
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
    adapterInfo( "Stored time value t = " + std::to_string(runTime_.value()), "debug" );
}

void preciceAdapter::Adapter::reloadCheckpointTime()
{
    const_cast<Time&>(runTime_).setTime( couplingIterationTimeValue_, couplingIterationTimeIndex_ );
    adapterInfo( "Reloaded time value t = " + std::to_string(runTime_.value()), "debug" );
}

void preciceAdapter::Adapter::setupCheckpointing()
{
    // Add fields in the checkpointing list
    adapterInfo( "Creating a list of checkpointed fields...", "debug" );

    /* Find and add all the registered objects in the mesh_
       of type volScalarField
    */

    // Print the available objects of type volScalarField
    // TODO Direct this through adapterInfo()
    adapterInfo( "Available objects of type volScalarField : ", "debug" );
    if ( ADAPTER_DEBUG_MODE ) {
        Info << mesh_.lookupClass<volScalarField>() << nl << nl;
    }

    wordList objectNames_ = mesh_.lookupClass<volScalarField>().toc();

    forAll( objectNames_, i )
    {
        if ( mesh_.foundObject<volScalarField>(objectNames_[i]) )
        {
            addCheckpointField(
                const_cast<volScalarField&>(
                    mesh_.lookupObject<volScalarField>( objectNames_[i] )
                )
            );

            adapterInfo(
                "Added " + objectNames_[i] + " in the list of checkpointed fields.",
                "debug"
            );
        }
        else
        {
            adapterInfo( "Did not find " + objectNames_[i], "debug" );
        }
    }

    /* Find and add all the registered objects in the mesh_
       of type volVectorField
    */

    // Print the available objects of type volVectorField
    // TODO Direct this through adapterInfo()
    adapterInfo( "Available objects of type volVectorField : ", "debug" );
    if ( ADAPTER_DEBUG_MODE ) {
        Info << mesh_.lookupClass<volVectorField>() << nl << nl;
    }

    objectNames_ = mesh_.lookupClass<volVectorField>().toc();

    forAll( objectNames_, i )
    {
        if ( mesh_.foundObject<volVectorField>(objectNames_[i]) )
        {
            addCheckpointField(
                const_cast<volVectorField&>(
                    mesh_.lookupObject<volVectorField>( objectNames_[i] )
                )
            );

            adapterInfo(
                "Added " + objectNames_[i] + " in the list of checkpointed fields.",
                "debug"
            );
        }
        else
        {
            adapterInfo( "Did not find " + objectNames_[i], "debug" );
        }
    }

    /* Find and add all the registered objects in the mesh_
       of type surfaceScalarField
    */

    // Print the available objects of type surfaceScalarField
    // TODO Direct this through adapterInfo()
    adapterInfo( "Available objects of type surfaceScalarField : ", "debug" );
    if ( ADAPTER_DEBUG_MODE ) {
        Info << mesh_.lookupClass<surfaceScalarField>() << nl << nl;
    }

    objectNames_ = mesh_.lookupClass<surfaceScalarField>().toc();

    forAll( objectNames_, i )
    {
        if ( mesh_.foundObject<surfaceScalarField>(objectNames_[i]) )
        {
            addCheckpointField(
                const_cast<surfaceScalarField&>(
                    mesh_.lookupObject<surfaceScalarField>( objectNames_[i] )
                )
            );

            adapterInfo(
                "Added " + objectNames_[i] + " in the list of checkpointed fields.",
                "debug"
            );
        }
        else
        {
            adapterInfo( "Did not find " + objectNames_[i], "debug" );
        }
    }

    // TODO Add other types, if needed.
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
    adapterInfo( "Reading a checkpoint...", "debug" );

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

    adapterInfo( "Checkpoint was read. Time = " + std::to_string(runTime_.value()), "debug" );
}

void preciceAdapter::Adapter::writeCheckpoint()
{
    adapterInfo( "Writing a checkpoint...", "debug" );

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
    adapterInfo( "Checkpoint for time t = " + std::to_string(runTime_.value()) + " was stored.", "debug" );
}

void preciceAdapter::Adapter::end()
{
    // Throw a warning if the simulation exited before the coupling was complete
    if ( NULL != precice_ && isCouplingOngoing() )
    {
        adapterInfo("The solver exited before the coupling was complete.", "warning");
    }
}

void preciceAdapter::Adapter::teardown()
{
    // If the solver interface was not deleted before, delete it now.
    // Normally it should be deleted when isCouplingOngoing() becomes false.
    if ( NULL != precice_ )
    {
        adapterInfo( "Destroying the preCICE solver interface...", "debug" );
        delete precice_;
        precice_ = NULL;
    }

    // Delete the preCICE solver interfaces
    if ( interfaces_.size() > 0 )
    {
        adapterInfo( "Deleting the interfaces...", "debug" );

        for ( uint i = 0; i < interfaces_.size(); i++ )
        {
            delete interfaces_.at( i );
        }
        interfaces_.clear();
    }

    // Delete the copied fields for checkpointing
    if ( checkpointing_ )
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

        checkpointing_ = false;
    }

}

preciceAdapter::Adapter::~Adapter()
{
    teardown();
}
