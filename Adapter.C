#include "Adapter.H"
#include "Interface.H"
#include "Utilities.H"

#include "IOstreams.H"

using namespace Foam;

preciceAdapter::Adapter::Adapter(const Time& runTime, const fvMesh& mesh)
:
runTime_(runTime),
mesh_(mesh)
{
    adapterInfo("The preciceAdapter was loaded.", "info");

    #ifdef ADAPTER_DEBUG_MODE
        Info<< "Registered objects: " << mesh_.names() << endl;
    #endif

    return;
}

bool preciceAdapter::Adapter::configFileCheck(const std::string adapterConfigFileName)
{
    DEBUG(adapterInfo("Checking the adapter's YAML configuration file..."));

    bool configErrors = false;

    YAML::Node adapterConfig = YAML::LoadFile(adapterConfigFileName);

    // TODO Consider simplifying
    // Check if the "participant" node exists
    if (!adapterConfig["participant"])
    {
        adapterInfo("The 'participant' node is missing in " + adapterConfigFileName + ".", "warning");
        configErrors = true;
    }

    // Check if the "precice-config-file" node exists
    if (!adapterConfig["precice-config-file"])
    {
        adapterInfo("The 'precice-config-file' node is missing in " + adapterConfigFileName + ".", "warning");
        configErrors = true;
        // TODO Check if the specified file exists
    }

    // Check if the "interfaces" node exists
    if (!adapterConfig["interfaces"])
    {
        adapterInfo("The 'interfaces' node is missing in " + adapterConfigFileName + ".", "warning");
        configErrors = true;
    }
    else
    {
        for (uint i = 0; i < adapterConfig["interfaces"].size(); i++)
        {
            if (!adapterConfig["interfaces"][i]["mesh"])
            {
                adapterInfo("The 'mesh' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning");
                configErrors = true;
            }
            if (!adapterConfig["interfaces"][i]["patches"])
            {
                adapterInfo("The 'patches' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning");
                configErrors = true;
            }
            if (!adapterConfig["interfaces"][i]["write-data"])
            {
                adapterInfo("The 'write-data' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning");
                configErrors = true;
                // TODO Add check for allowed values.
            }
            if (!adapterConfig["interfaces"][i]["read-data"])
            {
                adapterInfo("The 'read-data' node is missing for the interface #" + std::to_string(i+1) + " in " + adapterConfigFileName + ".", "warning");
                configErrors = true;
                // TODO Add check for allowed values.
            }
        }
    }

    return !configErrors;
}


bool preciceAdapter::Adapter::configFileRead()
{
    // Check the configuration file.
    // The file should be named "precice-adapter-config.yml" and located
    // in the global case directory. In case of a decomposed case, this is
    // the parent directory of the "processor*" directories.
    std::string adapterConfigFileName;
    if (runTime_.processorCase())
    {
        adapterConfigFileName = runTime_.path() + "/../precice-adapter-config.yml";
    }
    else
    {
        adapterConfigFileName = runTime_.path() + "/precice-adapter-config.yml";
    }
    adapterInfo("Reading the adapter's YAML configuration file " + adapterConfigFileName + "...", "info");

    if (!configFileCheck(adapterConfigFileName)) return false;

    // Load the YAML file
    YAML::Node adapterConfig_ = YAML::LoadFile(adapterConfigFileName);

    // Read the preCICE participant name
    participantName_ = adapterConfig_["participant"].as<std::string>();
    DEBUG(adapterInfo("  participant : " + participantName_));

    // Read the preCICE configuration file name
    preciceConfigFilename_ = adapterConfig_["precice-config-file"].as<std::string>();
    DEBUG(adapterInfo("  precice-config-file : " + preciceConfigFilename_));

    // TODO Read the coupling interfaces configuration
    YAML::Node adapterConfigInterfaces = adapterConfig_["interfaces"];
    DEBUG(adapterInfo("  interfaces : "));
    for (uint i = 0; i < adapterConfigInterfaces.size(); i++)
    {
        struct InterfaceConfig interfaceConfig;
        interfaceConfig.meshName = adapterConfigInterfaces[i]["mesh"].as<std::string>();
        DEBUG(adapterInfo("  - mesh      : " + interfaceConfig.meshName));

        DEBUG(adapterInfo("    patches   : "));
        for (uint j = 0; j < adapterConfigInterfaces[i]["patches"].size(); j++)
        {
            interfaceConfig.patchNames.push_back(adapterConfigInterfaces[i]["patches"][j].as<std::string>());
            DEBUG(adapterInfo("      " + adapterConfigInterfaces[i]["patches"][j].as<std::string>()));
        }

        // TODO: Consider simplification
        if (adapterConfigInterfaces[i]["write-data"])
        {
            DEBUG(adapterInfo("    write-data : "));
            if (adapterConfigInterfaces[i]["write-data"].size() > 0)
            {
                // TODO Check: before it was adapterConfigInterfaces[i]["read-data"].size()
                for (uint j = 0; j < adapterConfigInterfaces[i]["write-data"].size(); j++)
                {
                    interfaceConfig.writeData.push_back(adapterConfigInterfaces[i]["write-data"][j].as<std::string>());
                    DEBUG(adapterInfo("      " + adapterConfigInterfaces[i]["write-data"][j].as<std::string>()));
                }
            }
            else
            {
                interfaceConfig.writeData.push_back(adapterConfigInterfaces[i]["write-data"].as<std::string>());
                DEBUG(adapterInfo("      " + adapterConfigInterfaces[i]["write-data"].as<std::string>()));
            }
        }

        // TODO: Consider simplification
        if (adapterConfigInterfaces[i]["read-data"])
        {
            DEBUG(adapterInfo("    read-data : "));
            if (adapterConfigInterfaces[i]["read-data"].size() > 0)
            {
                for (uint j = 0; j < adapterConfigInterfaces[i]["read-data"].size(); j++)
                {
                    interfaceConfig.readData.push_back(adapterConfigInterfaces[i]["read-data"][j].as<std::string>());
                    DEBUG(adapterInfo("      " + adapterConfigInterfaces[i]["read-data"][j].as<std::string>()));
                }
            }
            else
            {
                interfaceConfig.readData.push_back(adapterConfigInterfaces[i]["read-data"].as<std::string>());
                DEBUG(adapterInfo("      " + adapterConfigInterfaces[i]["read-data"].as<std::string>()));
            }
        }

        interfacesConfig_.push_back(interfaceConfig);
    }

    // Set the subcyclingAllowed_ switch
    if (adapterConfig_["subcycling"])
    {
        subcyclingAllowed_ = adapterConfig_["subcycling"].as<bool>();
    }
    DEBUG(adapterInfo("    subcycling : " + std::to_string(subcyclingAllowed_)));

    // Set the preventEarlyExit_ switch
    if (adapterConfig_["preventEarlyExit"])
    {
        preventEarlyExit_ = adapterConfig_["preventEarlyExit"].as<bool>();
    }
    DEBUG(adapterInfo("    prevent early exit : " + std::to_string(preventEarlyExit_)));

    // Set the CHTenabled_ switch
    if (adapterConfig_["CHTenabled"])
    {
        CHTenabled_ = adapterConfig_["CHTenabled"].as<bool>();
    }
    DEBUG(adapterInfo("    CHT module enabled : " + std::to_string(CHTenabled_)));

    // If the CHT module is enabled, read create it, read the
    // CHT-specific options and configure it.
    if (CHTenabled_)
    {
        CHT_ = new CHT::ConjugateHeatTransfer(mesh_);
        if (!CHT_->configure(adapterConfig_)) return false;
    }
    else
    {
        adapterInfo("Only the CHT module is currently available. It cannot be disabled.", "warning");
        return false;
    }

    // NOTE: Create your module and read any options specific to it here

    return true;
}

void preciceAdapter::Adapter::configure()
{
    // Read the adapter's configuration file
    if (!configFileRead())
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

try{
    // Check the timestep type (fixed vs adjustable)
    DEBUG(adapterInfo("Checking the timestep type (fixed vs adjustable)..."));
    adjustableTimestep_ = runTime_.controlDict().lookupOrDefault("adjustTimeStep", false);

    if (adjustableTimestep_) {
        DEBUG(adapterInfo("  Timestep type: adjustable."));
    } else {
        DEBUG(adapterInfo("  Timestep type: fixed."));
    }

    // Initialize preCICE
    DEBUG(adapterInfo("Creating the preCICE solver interface..."));
    int MPIEnabled = 0;
    int MPIRank = 0;
    int MPISize = 1;

    MPI_Initialized(&MPIEnabled);
    DEBUG(adapterInfo("  MPI used: " + std::to_string(MPIEnabled)));

    if (MPIEnabled)
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &MPIRank);
        DEBUG(adapterInfo("  MPI rank: " + std::to_string(MPIRank)));

        MPI_Comm_size(MPI_COMM_WORLD, &MPISize);
        DEBUG(adapterInfo("  MPI size: " + std::to_string(MPISize)));
    }

    precice_ = new precice::SolverInterface(participantName_, MPIRank, MPISize);
    DEBUG(adapterInfo("  preCICE solver interface was created."));

    DEBUG(adapterInfo("Configuring preCICE..."));
    precice_->configure(preciceConfigFilename_);
    DEBUG(adapterInfo("  preCICE was configured."));

    // Create interfaces
    DEBUG(adapterInfo("Creating interfaces..."));
    for (uint i = 0; i < interfacesConfig_.size(); i++)
    {
        Interface * interface = new Interface(*precice_, mesh_, interfacesConfig_.at(i).meshName, interfacesConfig_.at(i).patchNames);
        interfaces_.push_back(interface);
        DEBUG(adapterInfo("Interface created on mesh" + interfacesConfig_.at(i).meshName));

        DEBUG(adapterInfo("Adding coupling data writers..."));
        for (uint j = 0; j < interfacesConfig_.at(i).writeData.size(); j++)
        {
            std::string dataName = interfacesConfig_.at(i).writeData.at(j);

            // Add CHT-related coupling data writers
            if (CHTenabled_)
            {
                CHT_->addWriters(dataName, interface);
            }

            // NOTE: Add any coupling data writers for your module here.
        } // end add coupling data writers

        DEBUG(adapterInfo("Adding coupling data readers..."));
        for (uint j = 0; j < interfacesConfig_.at(i).readData.size(); j++)
        {
            std::string dataName = interfacesConfig_.at(i).readData.at(j);

            // Add CHT-related coupling data readers
            if (CHTenabled_)
            {
                CHT_->addReaders(dataName, interface);
            }

            // NOTE: Add any coupling data readers for your module here.
        } // end add coupling data readers
    }

    // Initialize preCICE and exchange the first coupling data
    initialize();

    // Read the received coupling data
    readCouplingData();

    // If checkpointing is required, specify the checkpointed fields
    // and write the first checkpoint
    if (isWriteCheckpointRequired())
    {
        checkpointing_ = true;

        // Setup the checkpointing (find and add fields to checkpoint)
        setupCheckpointing();

        // Write checkpoint (for the first iteration)
        writeCheckpoint();
        fulfilledWriteCheckpoint();
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_)
    {
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
    if (preventEarlyExit_)
    {
        adapterInfo
        (
            "Setting the solver's endTime to infinity to prevent early exits. "
            "Only preCICE will control the simulation's endTime. "
            "Any functionObject's end() method will be triggered by the adapter. "
            "You may disable this behavior in the adapter's configuration.",
            "warning"
       );
        const_cast<Time&>(runTime_).setEndTime(GREAT);
    }

} catch (Foam::error) {
    errorsInConfigure = true;
}

    return;
}

void preciceAdapter::Adapter::execute()
{
    if (errorsInConfigure)
    {
        // Handle any errors during configure().
        // See the comments in configure() for details.
        adapterInfo
        (
            "There was a problem while configuring the adapter. "
            "See the log for details.",
            "error"
       );
    }

    // The solver has already solved the equations for this timestep.
    // Now call the adapter's methods to perform the coupling.

    // Write the coupling data in the buffer
    writeCouplingData();

    // Advance preCICE
    advance();

    // Read checkpoint if required
    if (isReadCheckpointRequired())
    {
        readCheckpoint();
        fulfilledReadCheckpoint();
    }

    // Read the received coupling data from the buffer
    readCouplingData();

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_)
    {
        adjustSolverTimeStep();
    }

    // Write checkpoint if required
    if (isWriteCheckpointRequired())
    {
        writeCheckpoint();
        fulfilledWriteCheckpoint();
    }

    // As soon as OpenFOAM writes the results, it will not try to write again
    // if the time takes the same value again. Therefore, during an implicit
    // coupling, we write again when the coupling timestep is complete.
    // Check the behavior e.g. by using watch on a result file:
    //     watch -n 0.1 -d ls --full-time Fluid/0.01/T.gz
    if (checkpointing_ && isCouplingTimestepComplete())
    {
        adapterInfo
        (
            "The coupling timestep completed. "
            "Writing the updated results.",
            "info"
       );
        const_cast<Time&>(runTime_).writeNow();
    }

    // If the coupling is not going to continue, tear down everything
    // and stop the simulation.
    if (!isCouplingOngoing())
    {
        adapterInfo("The coupling completed.", "info");

        // Finalize the preCICE solver interface and delete data
        finalize();

        // Tell OpenFOAM to stop the simulation.
        // Set the solver's endTime to now. The next evaluation of
        // runTime.run() will be false and the solver will exit.
        const_cast<Time&>(runTime_).setEndTime(runTime_.value());

        if (preventEarlyExit_)
        {
            adapterInfo
            (
                "The simulation was ended by preCICE. "
                "Calling the end() methods of any functionObject explicitly.",
                "info"
           );
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
    DEBUG(adapterInfo("Reading coupling data..."));

    for (uint i = 0; i < interfaces_.size(); i++)
    {
        interfaces_.at(i)->readCouplingData();
    }

    return;
}

void preciceAdapter::Adapter::writeCouplingData()
{
    DEBUG(adapterInfo("Writing coupling data..."));

    for (uint i = 0; i < interfaces_.size(); i++)
    {
        interfaces_.at(i)->writeCouplingData();
    }

    return;
}

void preciceAdapter::Adapter::initialize()
{
    DEBUG(adapterInfo("Iniializing the preCICE solver interface..."));
    timestepPrecice_ = precice_->initialize();

    preciceInitialized_ = true;

    if (precice_->isActionRequired(precice::constants::actionWriteInitialData()))
    {
        writeCouplingData();
        precice_->fulfilledAction(precice::constants::actionWriteInitialData());
    }

    DEBUG(adapterInfo("Initializing preCICE data..."));
    precice_->initializeData();

    adapterInfo("preCICE was configured and initialized", "info");

    return;
}

void preciceAdapter::Adapter::finalize()
{
    if (NULL != precice_ && preciceInitialized_ && !isCouplingOngoing())
    {
        DEBUG(adapterInfo("Finalizing the preCICE solver interface..."));

        // Finalize the preCICE solver interface
        precice_->finalize();

        preciceInitialized_ = false;

        // Delete the solver interface and all the related data
        teardown();
    }
    else
    {
        adapterInfo("Could not finalize preCICE.", "error");
    }

    return;
}

void preciceAdapter::Adapter::advance()
{
    adapterInfo("Advancing preCICE...", "info");

    timestepPrecice_ = precice_->advance(timestepSolver_);

    return;
}

void preciceAdapter::Adapter::adjustSolverTimeStep()
{
    DEBUG(adapterInfo("Adjusting the solver's timestep..."));

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
    if (!adjustableTimestep_)
    {
        // Have we already stored the timestep?
        if (!useStoredTimestep_)
        {
            // Show a warning if runTimeModifiable is set
            if (runTime_.runTimeModifiable())
            {
                adapterInfo
                (
                    "You have enabled 'runTimeModifiable' in the "
                    "controlDict. The preciceAdapter does not yet "
                    "fully support this functionality when "
                    "'adjustableTimestep' is not enabled. "
                    "If you modify the 'deltaT' in the controlDict "
                    "during the simulation, it will not be updated.",
                    "warning"
               );
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

    if (timestepSolverDetermined < timestepPrecice_)
    {
        if (!subcyclingAllowed_)
        {
            adapterInfo
            (
                "The solver's timestep cannot be smaller than the "
                "coupling timestep, because subcycling is disabled. ",
                "error"
           );
        }
        else
        {
            adapterInfo
            (
                "The solver's timestep is smaller than the "
                "coupling timestep. Subcycling...",
                "info"
           );
            timestepSolver_ = timestepSolverDetermined;
        }
    }
    else if (timestepSolverDetermined > timestepPrecice_)
    {
        adapterInfo
        (
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
        DEBUG(adapterInfo("The solver's timestep is the same as the "
                            "coupling timestep."));
        timestepSolver_ = timestepPrecice_;
    }

    // Update the solver's timestep (but don't trigger the adjustDeltaT(),
    // which also triggers the functionObject's adjustTimeStep()) (TODO)
    const_cast<Time&>(runTime_).setDeltaT(timestepSolver_, false);

    return;
}

bool preciceAdapter::Adapter::isCouplingOngoing()
{
    bool isCouplingOngoing = false;

    // If the coupling ends before the solver ends,
    // the solver would try to access this method again,
    // giving a segmentation fault if precice_
    // was not available.
    if (NULL != precice_)
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
    return precice_->isActionRequired(precice::constants::actionReadIterationCheckpoint());
}

bool preciceAdapter::Adapter::isWriteCheckpointRequired()
{
    return precice_->isActionRequired(precice::constants::actionWriteIterationCheckpoint());
}

void preciceAdapter::Adapter::fulfilledReadCheckpoint()
{
    precice_->fulfilledAction(precice::constants::actionReadIterationCheckpoint());

    return;
}

void preciceAdapter::Adapter::fulfilledWriteCheckpoint()
{
    precice_->fulfilledAction(precice::constants::actionWriteIterationCheckpoint());

    return;
}

void preciceAdapter::Adapter::storeCheckpointTime()
{
    couplingIterationTimeIndex_ = runTime_.timeIndex();
    couplingIterationTimeValue_ = runTime_.value();
    DEBUG(adapterInfo("Stored time value t = " + std::to_string(runTime_.value())));

    return;
}

void preciceAdapter::Adapter::reloadCheckpointTime()
{
    const_cast<Time&>(runTime_).setTime(couplingIterationTimeValue_, couplingIterationTimeIndex_);
    DEBUG(adapterInfo("Reloaded time value t = " + std::to_string(runTime_.value())));

    return;
}

void preciceAdapter::Adapter::setupCheckpointing()
{
    // Add fields in the checkpointing list
    DEBUG(adapterInfo("Creating a list of checkpointed fields..."));

    /* Find and add all the registered objects in the mesh_
       of type volScalarField
    */

    // Print the available objects of type volScalarField
    // TODO Direct this through adapterInfo()
    DEBUG(adapterInfo("Available objects of type volScalarField : "));
    #ifdef ADAPTER_DEBUG_MODE
        Info << mesh_.lookupClass<volScalarField>() << nl << nl;
    #endif

    wordList objectNames_ = mesh_.lookupClass<volScalarField>().toc();

    forAll(objectNames_, i)
    {
        if (mesh_.foundObject<volScalarField>(objectNames_[i]))
        {
            addCheckpointField
            (
                const_cast<volScalarField&>
                (
                    mesh_.lookupObject<volScalarField>(objectNames_[i])
               )
           );

            #ifdef ADAPTER_DEBUG_MODE
            adapterInfo
            (
                "Added " + objectNames_[i] +
                " in the list of checkpointed fields."
           );
            #endif

            // TODO: Known bug, see readCheckpoint()
            if ("epsilon" == objectNames_[i])
            {
                DEBUG(adapterInfo("Known bug: after reading a checkpoint, "
                        "the boundaries for epsilon will not be corrected.",
                        "warning"));
            }
        }
        else
        {
            adapterInfo("Could not checkpoint " + objectNames_[i], "warning");
        }
    }

    /* Find and add all the registered objects in the mesh_
       of type volVectorField
    */

    // Print the available objects of type volVectorField
    // TODO Direct this through adapterInfo()
    DEBUG(adapterInfo("Available objects of type volVectorField : "));
    #ifdef ADAPTER_DEBUG_MODE
        Info << mesh_.lookupClass<volVectorField>() << nl << nl;
    #endif

    objectNames_ = mesh_.lookupClass<volVectorField>().toc();

    forAll(objectNames_, i)
    {
        if (mesh_.foundObject<volVectorField>(objectNames_[i]))
        {
            addCheckpointField
            (
                const_cast<volVectorField&>
                (
                    mesh_.lookupObject<volVectorField>(objectNames_[i])
               )
           );

            #ifdef ADAPTER_DEBUG_MODE
            adapterInfo
            (
                "Added " + objectNames_[i] +
                " in the list of checkpointed fields."
           );
            #endif
        }
        else
        {
            adapterInfo("Could not checkpoint " + objectNames_[i], "warning");
        }
    }

    /* Find and add all the registered objects in the mesh_
       of type surfaceScalarField
    */

    #ifdef ADAPTER_DEBUG_MODE
        // Print the available objects of type surfaceScalarField
        adapterInfo("Available objects of type surfaceScalarField : ");
        // TODO Direct this through adapterInfo()
        Info << mesh_.lookupClass<surfaceScalarField>() << nl << nl;
    #endif

    objectNames_ = mesh_.lookupClass<surfaceScalarField>().toc();

    forAll(objectNames_, i)
    {
        if (mesh_.foundObject<surfaceScalarField>(objectNames_[i]))
        {
            addCheckpointField
            (
                const_cast<surfaceScalarField&>
                (
                    mesh_.lookupObject<surfaceScalarField>(objectNames_[i])
               )
           );

            #ifdef ADAPTER_DEBUG_MODE
            adapterInfo
            (
                "Added " + objectNames_[i] +
                " in the list of checkpointed fields."
           );
            #endif
        }
        else
        {
            adapterInfo("Could not checkpoint " + objectNames_[i], "warning");
        }
    }

    // NOTE: Add here other object types to checkpoint, if needed.

    return;
}

void preciceAdapter::Adapter::addCheckpointField(volScalarField & field)
{
    volScalarField * copy = new volScalarField(field);
    volScalarFields_.push_back(&field);
    volScalarFieldCopies_.push_back(copy);

    return;
}

void preciceAdapter::Adapter::addCheckpointField(volVectorField & field)
{
    volVectorField * copy = new volVectorField(field);
    volVectorFields_.push_back(&field);
    volVectorFieldCopies_.push_back(copy);

    return;
}

void preciceAdapter::Adapter::addCheckpointField(surfaceScalarField & field)
{
    surfaceScalarField * copy = new surfaceScalarField(field);
    surfaceScalarFields_.push_back(&field);
    surfaceScalarFieldCopies_.push_back(copy);

    return;
}

// NOTE: Add here methods to add other object types to checkpoint, if needed.

void preciceAdapter::Adapter::readCheckpoint()
{
    DEBUG(adapterInfo("Reading a checkpoint..."));

    // Reload the runTime
    reloadCheckpointTime();

    // Reload all the fields of type volScalarField
    for (uint i = 0; i < volScalarFields_.size(); i++)
    {
        // Load the volume field
        *(volScalarFields_.at(i)) == *(volScalarFieldCopies_.at(i));
        // Evaluate the boundaries, if supported
        try{
            if ("epsilon" != volScalarFields_.at(i)->name())
            {
                volScalarFields_.at(i)->correctBoundaryConditions();
            }
            // TODO: Known bug: cannot find "volScalarField::Internal kEpsilon:G"
            // Currently it is skipped. Before it was not corrected at all.
        } catch (Foam::error) {
            DEBUG(adapterInfo("Could not evaluate the boundary for" + volScalarFields_.at(i)->name(), "warning"));
        }

    }

    // Reload all the fields of type volVectorField
    for (uint i = 0; i < volVectorFields_.size(); i++)
    {
        // Load the volume field
        *(volVectorFields_.at(i)) == *(volVectorFieldCopies_.at(i));
        // Evaluate the boundaries
        try{
            volVectorFields_.at(i)->correctBoundaryConditions();
        } catch (...) {
            DEBUG(adapterInfo("Could not evaluate the boundary for" + volVectorFields_.at(i)->name(), "warning"));
        }
    }

    // Reload all the fields of type surfaceScalarField
    for (uint i = 0; i < surfaceScalarFields_.size(); i++)
    {
        *(surfaceScalarFields_.at(i)) == *(surfaceScalarFieldCopies_.at(i));
    }

    // TODO Reload all the fields of type surfaceVectorField

    // NOTE: Add here other field types to read, if needed.

    #ifdef ADAPTER_DEBUG_MODE
        adapterInfo
        (
            "Checkpoint was read. Time = " + std::to_string(runTime_.value())
       );
    #endif

    return;
}

void preciceAdapter::Adapter::writeCheckpoint()
{
    DEBUG(adapterInfo("Writing a checkpoint..."));

    // Store the runTime
    storeCheckpointTime();

    // Store all the fields of type volScalarField
    for (uint i = 0; i < volScalarFields_.size(); i++)
    {
        *(volScalarFieldCopies_.at(i)) == *(volScalarFields_.at(i));
    }

    // Store all the fields of type volVectorField
    for (uint i = 0; i < volVectorFields_.size(); i++)
    {
        *(volVectorFieldCopies_.at(i)) == *(volVectorFields_.at(i));
    }

    // Store all the fields of type surfaceScalarField
    for (uint i = 0; i < surfaceScalarFields_.size(); i++)
    {
        *(surfaceScalarFieldCopies_.at(i)) == *(surfaceScalarFields_.at(i));
    }

    // TODO Store all the fields of type surfaceVectorField

    // NOTE: Add here other types to write, if needed.

    #ifdef ADAPTER_DEBUG_MODE
        adapterInfo
        (
            "Checkpoint for time t = " + std::to_string(runTime_.value()) +
            " was stored."
       );
    #endif

    return;
}

void preciceAdapter::Adapter::end()
{
    // Throw a warning if the simulation exited before the coupling was complete
    if (NULL != precice_ && isCouplingOngoing())
    {
        adapterInfo("The solver exited before the coupling was complete.", "warning");
    }

    return;
}

void preciceAdapter::Adapter::teardown()
{
    // If the solver interface was not deleted before, delete it now.
    // Normally it should be deleted when isCouplingOngoing() becomes false.
    if (NULL != precice_)
    {
        DEBUG(adapterInfo("Destroying the preCICE solver interface..."));
        delete precice_;
        precice_ = NULL;
    }

    // Delete the preCICE solver interfaces
    if (interfaces_.size() > 0)
    {
        DEBUG(adapterInfo("Deleting the interfaces..."));
        for (uint i = 0; i < interfaces_.size(); i++)
        {
            delete interfaces_.at(i);
        }
        interfaces_.clear();
    }

    // Delete the copied fields for checkpointing
    if (checkpointing_)
    {
        DEBUG(adapterInfo("Deleting the checkpoints... "));
        for (uint i = 0; i < volScalarFieldCopies_.size(); i++)
        {
            delete volScalarFieldCopies_.at(i);
        }
        volScalarFieldCopies_.clear();

        for (uint i = 0; i < volVectorFieldCopies_.size(); i++)
        {
            delete volVectorFieldCopies_.at(i);
        }
        volVectorFieldCopies_.clear();

        for (uint i = 0; i < surfaceScalarFieldCopies_.size(); i++)
        {
            delete surfaceScalarFieldCopies_.at(i);
        }
        surfaceScalarFieldCopies_.clear();

        // NOTE: Add here delete for other types, if needed

        checkpointing_ = false;
    }

    // Delete the CHT module
    if(NULL != CHT_)
    {
        DEBUG(adapterInfo("Destroying the CHT module..."));
        delete CHT_;
        CHT_ = NULL;
    }

    return;
}

preciceAdapter::Adapter::~Adapter()
{
    teardown();

    return;
}
