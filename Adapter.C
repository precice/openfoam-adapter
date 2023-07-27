#include "Adapter.H"
#include "Interface.H"
#include "Utilities.H"

#include "IOstreams.H"

using namespace Foam;

preciceAdapter::Adapter::Adapter(const Time& runTime, const fvMesh& mesh)
: runTime_(runTime),
  mesh_(mesh)
{
    adapterInfo("Loaded the OpenFOAM-preCICE adapter - v1.2.3 + unreleased changes.", "info");

    return;
}

bool preciceAdapter::Adapter::configFileRead()
{

    // We need a try-catch here, as if reading preciceDict fails,
    // the respective exception will be reduced to a warning.
    // See also comment in preciceAdapter::Adapter::configure().
    try
    {
        SETUP_TIMER();
        adapterInfo("Reading preciceDict...", "info");

        // TODO: static is just a quick workaround to be able
        // to find the dictionary also out of scope (e.g. in KappaEffective).
        // We need a better solution.
        static IOdictionary preciceDict(
            IOobject(
                "preciceDict",
                runTime_.system(),
                mesh_,
                IOobject::MUST_READ_IF_MODIFIED,
                IOobject::NO_WRITE));

        // Read and display the preCICE configuration file name
        preciceConfigFilename_ = preciceDict.get<fileName>("preciceConfig");
        DEBUG(adapterInfo("  precice-config-file : " + preciceConfigFilename_));

        // Read and display the participant name
        participantName_ = preciceDict.get<word>("participant");
        DEBUG(adapterInfo("  participant name    : " + participantName_));

        // Read and display the list of modules
        DEBUG(adapterInfo("  modules requested   : "));
        auto modules_ = preciceDict.get<wordList>("modules");
        for (const auto& module : modules_)
        {
            DEBUG(adapterInfo("  - " + module + "\n"));

            // Set the modules switches
            if (module == "CHT")
            {
                CHTenabled_ = true;
            }

            if (module == "FSI")
            {
                FSIenabled_ = true;
            }

            if (module == "FF")
            {
                FFenabled_ = true;
            }
        }

        // Every interface is a subdictionary of "interfaces",
        // each with an arbitrary name. Read all of them and create
        // a list (here: pointer) of dictionaries.
        const auto* interfaceDictPtr = preciceDict.findDict("interfaces");
        DEBUG(adapterInfo("  interfaces : "));

        // Check if we found any interfaces
        // and get the details of each interface
        if (!interfaceDictPtr)
        {
            adapterInfo("  Empty list of interfaces", "warning");
            return false;
        }
        else
        {
            for (const entry& interfaceDictEntry : *interfaceDictPtr)
            {
                if (interfaceDictEntry.isDict())
                {
                    const dictionary& interfaceDict = interfaceDictEntry.dict();
                    struct InterfaceConfig interfaceConfig;

                    interfaceConfig.meshName = interfaceDict.get<word>("mesh");
                    DEBUG(adapterInfo("  - mesh         : " + interfaceConfig.meshName));

                    // By default, assume "faceCenters" as locationsType
                    interfaceConfig.locationsType = interfaceDict.lookupOrDefault<word>("locations", "faceCenters");
                    DEBUG(adapterInfo("    locations    : " + interfaceConfig.locationsType));

                    // By default, assume that no mesh connectivity is required (i.e. no nearest-projection mapping)
                    interfaceConfig.meshConnectivity = interfaceDict.lookupOrDefault<bool>("connectivity", false);
                    // Mesh connectivity only makes sense in case of faceNodes, check and raise a warning otherwise
                    if (interfaceConfig.meshConnectivity && interfaceConfig.locationsType == "faceCenters")
                    {
                        DEBUG(adapterInfo("Mesh connectivity is not supported for faceCenters. \n"
                                          "Please configure the desired interface with the locationsType faceNodes. \n"
                                          "Have a look in the adapter documentation for detailed information.",
                                          "warning"));
                        return false;
                    }
                    DEBUG(adapterInfo("    connectivity : " + std::to_string(interfaceConfig.meshConnectivity)));

                    DEBUG(adapterInfo("    patches      : "));
                    auto patches = interfaceDict.get<wordList>("patches");
                    for (auto patch : patches)
                    {
                        interfaceConfig.patchNames.push_back(patch);
                        DEBUG(adapterInfo("      - " + patch));
                    }

                    DEBUG(adapterInfo("    writeData    : "));
                    auto writeData = interfaceDict.get<wordList>("writeData");
                    for (auto writeDatum : writeData)
                    {
                        interfaceConfig.writeData.push_back(writeDatum);
                        DEBUG(adapterInfo("      - " + writeDatum));
                    }

                    DEBUG(adapterInfo("    readData     : "));
                    auto readData = interfaceDict.get<wordList>("readData");
                    for (auto readDatum : readData)
                    {
                        interfaceConfig.readData.push_back(readDatum);
                        DEBUG(adapterInfo("      - " + readDatum));
                    }
                    interfacesConfig_.push_back(interfaceConfig);
                }
            }
        }

        // NOTE: set the switch for your new module here

        // If the CHT module is enabled, create it, read the
        // CHT-specific options and configure it.
        if (CHTenabled_)
        {
            CHT_ = new CHT::ConjugateHeatTransfer(mesh_);
            if (!CHT_->configure(preciceDict))
            {
                return false;
            }
        }

        // If the FSI module is enabled, create it, read the
        // FSI-specific options and configure it.
        if (FSIenabled_)
        {
            // Check for unsupported FSI with meshConnectivity
            for (uint i = 0; i < interfacesConfig_.size(); i++)
            {
                if (interfacesConfig_.at(i).meshConnectivity == true)
                {
                    adapterInfo(
                        "You have requested mesh connectivity (most probably for nearest-projection mapping) "
                        "and you have enabled the FSI module. "
                        "Mapping with connectivity information is not implemented for FSI, only for CHT-related fields. "
                        "warning");
                    return false;
                }
            }

            FSI_ = new FSI::FluidStructureInteraction(mesh_, runTime_);
            if (!FSI_->configure(preciceDict))
            {
                return false;
            }
        }

        if (FFenabled_)
        {
            FF_ = new FF::FluidFluid(mesh_);
            if (!FF_->configure(preciceDict))
            {
                return false;
            }
        }

        // NOTE: Create your module and read any options specific to it here

        if (!CHTenabled_ && !FSIenabled_ && !FFenabled_) // NOTE: Add your new switch here
        {
            adapterInfo("No module is enabled.", "error-deferred");
            return false;
        }

        // TODO: Loading modules should be implemented in more general way,
        // in order to avoid code duplication. See issue #16 on GitHub.

        ACCUMULATE_TIMER(timeInConfigRead_);
    }
    catch (const Foam::error& e)
    {
        adapterInfo(e.message(), "error-deferred");
        return false;
    }

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

    try
    {
        // Check the timestep type (fixed vs adjustable)
        DEBUG(adapterInfo("Checking the timestep type (fixed vs adjustable)..."));
        adjustableTimestep_ = runTime_.controlDict().lookupOrDefault("adjustTimeStep", false);

        if (adjustableTimestep_)
        {
            DEBUG(adapterInfo("  Timestep type: adjustable."));
        }
        else
        {
            DEBUG(adapterInfo("  Timestep type: fixed."));
        }

        // Construct preCICE
        SETUP_TIMER();
        DEBUG(adapterInfo("Creating the preCICE solver interface..."));
        DEBUG(adapterInfo("  Number of processes: " + std::to_string(Pstream::nProcs())));
        DEBUG(adapterInfo("  MPI rank: " + std::to_string(Pstream::myProcNo())));
        precice_ = new precice::Participant(participantName_, preciceConfigFilename_, Pstream::myProcNo(), Pstream::nProcs());
        DEBUG(adapterInfo("  preCICE solver interface was created."));

        ACCUMULATE_TIMER(timeInPreciceConstruct_);

        // Create interfaces
        REUSE_TIMER();
        DEBUG(adapterInfo("Creating interfaces..."));
        for (uint i = 0; i < interfacesConfig_.size(); i++)
        {
            std::string namePointDisplacement = FSIenabled_ ? FSI_->getPointDisplacementFieldName() : "default";
            std::string nameCellDisplacement = FSIenabled_ ? FSI_->getCellDisplacementFieldName() : "default";
            bool restartFromDeformed = FSIenabled_ ? FSI_->isRestartingFromDeformed() : false;

            Interface* interface = new Interface(*precice_, mesh_, interfacesConfig_.at(i).meshName, interfacesConfig_.at(i).locationsType, interfacesConfig_.at(i).patchNames, interfacesConfig_.at(i).meshConnectivity, restartFromDeformed, namePointDisplacement, nameCellDisplacement);
            interfaces_.push_back(interface);
            DEBUG(adapterInfo("Interface created on mesh " + interfacesConfig_.at(i).meshName));

            DEBUG(adapterInfo("Adding coupling data writers..."));
            for (uint j = 0; j < interfacesConfig_.at(i).writeData.size(); j++)
            {
                std::string dataName = interfacesConfig_.at(i).writeData.at(j);

                unsigned int inModules = 0;

                // Add CHT-related coupling data writers
                if (CHTenabled_ && CHT_->addWriters(dataName, interface))
                {
                    inModules++;
                }

                // Add FSI-related coupling data writers
                if (FSIenabled_ && FSI_->addWriters(dataName, interface))
                {
                    inModules++;
                }

                // Add FF-related coupling data writers
                if (FFenabled_ && FF_->addWriters(dataName, interface))
                {
                    inModules++;
                }

                if (inModules == 0)
                {
                    adapterInfo("I don't know how to write \"" + dataName
                                    + "\". Maybe this is a typo or maybe you need to enable some adapter module?",
                                "error-deferred");
                }
                else if (inModules > 1)
                {
                    adapterInfo("It looks like more than one modules can write \"" + dataName
                                    + "\" and I don't know how to choose. Try disabling one of the modules.",
                                "error-deferred");
                }

                // NOTE: Add any coupling data writers for your module here.
            } // end add coupling data writers

            DEBUG(adapterInfo("Adding coupling data readers..."));
            for (uint j = 0; j < interfacesConfig_.at(i).readData.size(); j++)
            {
                std::string dataName = interfacesConfig_.at(i).readData.at(j);

                unsigned int inModules = 0;

                // Add CHT-related coupling data readers
                if (CHTenabled_ && CHT_->addReaders(dataName, interface)) inModules++;

                // Add FSI-related coupling data readers
                if (FSIenabled_ && FSI_->addReaders(dataName, interface)) inModules++;

                // Add FF-related coupling data readers
                if (FFenabled_ && FF_->addReaders(dataName, interface)) inModules++;

                if (inModules == 0)
                {
                    adapterInfo("I don't know how to read \"" + dataName
                                    + "\". Maybe this is a typo or maybe you need to enable some adapter module?",
                                "error-deferred");
                }
                else if (inModules > 1)
                {
                    adapterInfo("It looks like more than one modules can read \"" + dataName
                                    + "\" and I don't know how to choose. Try disabling one of the modules.",
                                "error-deferred");
                }

                // NOTE: Add any coupling data readers for your module here.
            } // end add coupling data readers

            // Create the interface's data buffer
            interface->createBuffer();
        }
        ACCUMULATE_TIMER(timeInMeshSetup_);

        // Initialize preCICE and exchange the first coupling data
        initialize();

        // If checkpointing is required, specify the checkpointed fields
        // and write the first checkpoint
        if (requiresWritingCheckpoint())
        {
            checkpointing_ = true;

            // Setup the checkpointing (find and add fields to checkpoint)
            setupCheckpointing();

            // Write checkpoint (for the first iteration)
            writeCheckpoint();
        }

        // Adjust the timestep for the first iteration, if it is fixed
        if (!adjustableTimestep_)
        {
            adjustSolverTimeStepAndReadData();
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
        adapterInfo(
            "Setting the solver's endTime to infinity to prevent early exits. "
            "Only preCICE will control the simulation's endTime. "
            "Any functionObject's end() method will be triggered by the adapter. "
            "You may disable this behavior in the adapter's configuration.",
            "info");
        const_cast<Time&>(runTime_).setEndTime(GREAT);
    }
    catch (const Foam::error& e)
    {
        adapterInfo(e.message(), "error-deferred");
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
        adapterInfo(
            "There was a problem while configuring the adapter. "
            "See the log for details.",
            "error");
    }

    // The solver has already solved the equations for this timestep.
    // Now call the adapter's methods to perform the coupling.

    // TODO add a function which checks if all fields are checkpointed.
    // if (ncheckpointed is nregisterdobjects. )

    // Write the coupling data in the buffer
    writeCouplingData();

    // Advance preCICE
    advance();

    // Read checkpoint if required
    if (requiresReadingCheckpoint())
    {
        readCheckpoint();
    }

    // Write checkpoint if required
    if (requiresWritingCheckpoint())
    {
        writeCheckpoint();
    }

    // As soon as OpenFOAM writes the results, it will not try to write again
    // if the time takes the same value again. Therefore, during an implicit
    // coupling, we write again when the coupling timestep is complete.
    // Check the behavior e.g. by using watch on a result file:
    //     watch -n 0.1 -d ls --full-time Fluid/0.01/T.gz
    SETUP_TIMER();
    if (checkpointing_ && isCouplingTimeWindowComplete())
    {
        // Check if the time directory already exists
        // (i.e. the solver wrote results that need to be updated)
        if (runTime_.timePath().type() == fileName::DIRECTORY)
        {
            adapterInfo(
                "The coupling timestep completed. "
                "Writing the updated results.",
                "info");
            const_cast<Time&>(runTime_).writeNow();
        }
    }
    ACCUMULATE_TIMER(timeInWriteResults_);

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_)
    {
        adjustSolverTimeStepAndReadData();
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
        adapterInfo(
            "The simulation was ended by preCICE. "
            "Calling the end() methods of any functionObject explicitly.",
            "info");
        adapterInfo("Great that you are using the OpenFOAM-preCICE adapter! "
                    "Next to the preCICE library and any other components, please also cite this adapter. "
                    "Find how on https://precice.org/adapter-openfoam-overview.html.",
                    "info");
        const_cast<Time&>(runTime_).functionObjects().end();
    }

    return;
}


void preciceAdapter::Adapter::adjustTimeStep()
{
    adjustSolverTimeStepAndReadData();

    return;
}

void preciceAdapter::Adapter::readCouplingData(double relativeReadTime)
{
    SETUP_TIMER();
    DEBUG(adapterInfo("Reading coupling data..."));

    for (uint i = 0; i < interfaces_.size(); i++)
    {
        interfaces_.at(i)->readCouplingData(relativeReadTime);
    }

    ACCUMULATE_TIMER(timeInRead_);

    return;
}

void preciceAdapter::Adapter::writeCouplingData()
{
    SETUP_TIMER();
    DEBUG(adapterInfo("Writing coupling data..."));

    for (uint i = 0; i < interfaces_.size(); i++)
    {
        interfaces_.at(i)->writeCouplingData();
    }

    ACCUMULATE_TIMER(timeInWrite_);

    return;
}

void preciceAdapter::Adapter::initialize()
{
    DEBUG(adapterInfo("Initializing the preCICE solver interface..."));
    SETUP_TIMER();

    if (precice_->requiresInitialData())
        writeCouplingData();

    DEBUG(adapterInfo("Initializing preCICE data..."));
    precice_->initialize();
    timestepPrecice_ = precice_->getMaxTimeStepSize();
    preciceInitialized_ = true;
    ACCUMULATE_TIMER(timeInInitialize_);

    adapterInfo("preCICE was configured and initialized", "info");

    return;
}

void preciceAdapter::Adapter::finalize()
{
    if (NULL != precice_ && preciceInitialized_ && !isCouplingOngoing())
    {
        DEBUG(adapterInfo("Finalizing the preCICE solver interface..."));

        // Finalize the preCICE solver interface
        SETUP_TIMER();
        precice_->finalize();
        ACCUMULATE_TIMER(timeInFinalize_);

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
    DEBUG(adapterInfo("Advancing preCICE..."));

    SETUP_TIMER();
    precice_->advance(timestepSolver_);
    timestepPrecice_ = precice_->getMaxTimeStepSize();
    ACCUMULATE_TIMER(timeInAdvance_);

    return;
}

void preciceAdapter::Adapter::adjustSolverTimeStepAndReadData()
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
                adapterInfo(
                    "You have enabled 'runTimeModifiable' in the "
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

    if (timestepSolverDetermined < timestepPrecice_)
    {
        // Add a bool 'subCycling = true' which is checked in the storeMeshPoints() function.
        adapterInfo(
            "The solver's timestep is smaller than the "
            "coupling timestep. Subcycling...",
            "info");
        timestepSolver_ = timestepSolverDetermined;
        // TODO subcycling is enabled. For FSI the oldVolumes must be written, which is normally not done.
        if (FSIenabled_)
        {
            adapterInfo(
                "The adapter does not fully support subcycling for FSI and instabilities may occur.",
                "warning");
        }
    }
    else if (timestepSolverDetermined > timestepPrecice_)
    {
        adapterInfo(
            "The solver's timestep cannot be larger than the coupling timestep."
            " Adjusting from "
                + std::to_string(timestepSolverDetermined) + " to " + std::to_string(timestepPrecice_),
            "warning");
        timestepSolver_ = timestepPrecice_;
    }
    else
    {
        DEBUG(adapterInfo("The solver's timestep is the same as the "
                          "coupling timestep."));
        timestepSolver_ = timestepPrecice_;
    }

    // Update the solver's timestep (but don't trigger the adjustDeltaT(),
    // which also triggers the functionObject's adjustTimeStep())
    // TODO: Keep this in mind if any relevant problem appears.
    const_cast<Time&>(runTime_).setDeltaT(timestepSolver_, false);

    DEBUG(adapterInfo("Reading coupling data associated to the calculated time-step size..."));

    // Read the received coupling data from the buffer
    // Fits to an implicit Euler
    readCouplingData(runTime_.deltaT().value());

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

bool preciceAdapter::Adapter::isCouplingTimeWindowComplete()
{
    return precice_->isTimeWindowComplete();
}

bool preciceAdapter::Adapter::requiresReadingCheckpoint()
{
    return precice_->requiresReadingCheckpoint();
}

bool preciceAdapter::Adapter::requiresWritingCheckpoint()
{
    return precice_->requiresWritingCheckpoint();
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
    // TODO also reset the current iteration?!
    DEBUG(adapterInfo("Reloaded time value t = " + std::to_string(runTime_.value())));

    return;
}

void preciceAdapter::Adapter::storeMeshPoints()
{
    DEBUG(adapterInfo("Storing mesh points..."));
    // TODO: In foam-extend, we would need "allPoints()". Check if this gives the same data.
    meshPoints_ = mesh_.points();
    oldMeshPoints_ = mesh_.oldPoints();

    /*
    // TODO  This is only required for subcycling. It should not be called when not subcycling!!
    // Add a bool 'subcycling' which can be evaluated every timestep.
    if ( !oldVolsStored && mesh_.foundObject<volScalarField::Internal>("V00") ) // For Ddt schemes which use one previous timestep
    {
        setupMeshVolCheckpointing();
        oldVolsStored = true;
    }
    // Update any volume fields from the buffer to the checkpointed values (if already exists.)
    */

    DEBUG(adapterInfo("Stored mesh points."));
    if (mesh_.moving())
    {
        if (!meshCheckPointed)
        {
            // Set up the checkpoint for the mesh flux: meshPhi
            setupMeshCheckpointing();
            meshCheckPointed = true;
        }
        writeMeshCheckpoint();
        writeVolCheckpoint(); // Does not write anything unless subcycling.
    }
}

void preciceAdapter::Adapter::reloadMeshPoints()
{
    if (!mesh_.moving())
    {
        DEBUG(adapterInfo("Mesh points not moved as the mesh is not moving"));
        return;
    }

    // In Foam::polyMesh::movePoints.
    // TODO: The function movePoints overwrites the pointer to the old mesh.
    // Therefore, if you revert the mesh, the oldpointer will be set to the points, which are the new values.
    DEBUG(adapterInfo("Moving mesh points to their previous locations..."));

    // TODO
    // Switch oldpoints on for pure physics. (is this required?). Switch off for better mesh deformation capabilities?
    // const_cast<pointField&>(mesh_.points()) = oldMeshPoints_;
    const_cast<fvMesh&>(mesh_).movePoints(meshPoints_);

    DEBUG(adapterInfo("Moved mesh points to their previous locations."));

    // TODO The if statement can be removed in this case, but it is still included for clarity
    if (meshCheckPointed)
    {
        readMeshCheckpoint();
    }

    /*  // TODO This part should only be used when sybcycling. See the description in 'storeMeshPoints()'
        // The if statement can be removed in this case, but it is still included for clarity
    if ( oldVolsStored )
    {
        readVolCheckpoint();
    }
    */
}

void preciceAdapter::Adapter::setupMeshCheckpointing()
{
    // The other mesh <type>Fields:
    //      C
    //      Cf
    //      Sf
    //      magSf
    //      delta
    // are updated by the function fvMesh::movePoints. Only the meshPhi needs checkpointing.
    DEBUG(adapterInfo("Creating a list of the mesh checkpointed fields..."));

    // Add meshPhi to the checkpointed fields
    addMeshCheckpointField(
        const_cast<surfaceScalarField&>(
            mesh_.phi()));
#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Added " + mesh_.phi().name() + " in the list of checkpointed fields.");
#endif
}

void preciceAdapter::Adapter::setupMeshVolCheckpointing()
{
    DEBUG(adapterInfo("Creating a list of the mesh volume checkpointed fields..."));
    // Add the V0 and the V00 to the list of checkpointed fields.
    // For V0
    addVolCheckpointField(
        const_cast<volScalarField::Internal&>(
            mesh_.V0()));
#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Added " + mesh_.V0().name() + " in the list of checkpointed fields.");
#endif
    // For V00
    addVolCheckpointField(
        const_cast<volScalarField::Internal&>(
            mesh_.V00()));
#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Added " + mesh_.V00().name() + " in the list of checkpointed fields.");
#endif

    // Also add the buffer fields.
    // TODO For V0
    /* addVolCheckpointFieldBuffer
    (
        const_cast<volScalarField::Internal&>
        (
            mesh_.V0()
        )
    ); */
#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Added " + mesh_.V0().name() + " in the list of buffer checkpointed fields.");
#endif
    // TODO For V00
    /* addVolCheckpointFieldBuffer
    (
        const_cast<volScalarField::Internal&>
        (
            mesh_.V00()
        )
    );*/
#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Added " + mesh_.V00().name() + " in the list of buffer checkpointed fields.");
#endif
}


void preciceAdapter::Adapter::setupCheckpointing()
{
    SETUP_TIMER();

    // Add fields in the checkpointing list - sorted for parallel consistency
    DEBUG(adapterInfo("Adding in checkpointed fields..."));

#undef doLocalCode
#define doLocalCode(GeomField)                                           \
    /* Checkpoint registered GeomField objects */                        \
    for (const word& obj : mesh_.sortedNames<GeomField>())               \
    {                                                                    \
        addCheckpointField(mesh_.thisDb().getObjectPtr<GeomField>(obj)); \
        DEBUG(adapterInfo("Checkpoint " + obj + " : " #GeomField));      \
    }

    doLocalCode(volScalarField);
    doLocalCode(volVectorField);
    doLocalCode(volTensorField);
    doLocalCode(volSymmTensorField);

    doLocalCode(surfaceScalarField);
    doLocalCode(surfaceVectorField);
    doLocalCode(surfaceTensorField);

    doLocalCode(pointScalarField);
    doLocalCode(pointVectorField);
    doLocalCode(pointTensorField);

    // NOTE: Add here other object types to checkpoint, if needed.

#undef doLocalCode

    ACCUMULATE_TIMER(timeInCheckpointingSetup_);
}


// All mesh checkpointed fields

void preciceAdapter::Adapter::addMeshCheckpointField(surfaceScalarField& field)
{
    {
        meshSurfaceScalarFields_.push_back(&field);
        meshSurfaceScalarFieldCopies_.push_back(new surfaceScalarField(field));
    }
}

void preciceAdapter::Adapter::addMeshCheckpointField(surfaceVectorField& field)
{
    {
        meshSurfaceVectorFields_.push_back(&field);
        meshSurfaceVectorFieldCopies_.push_back(new surfaceVectorField(field));
    }
}

void preciceAdapter::Adapter::addMeshCheckpointField(volVectorField& field)
{
    {
        meshVolVectorFields_.push_back(&field);
        meshVolVectorFieldCopies_.push_back(new volVectorField(field));
    }
}

// TODO Internal field for the V0 (volume old) and V00 (volume old-old) fields
void preciceAdapter::Adapter::addVolCheckpointField(volScalarField::Internal& field)
{
    {
        volScalarInternalFields_.push_back(&field);
        volScalarInternalFieldCopies_.push_back(new volScalarField::Internal(field));
    }
}


void preciceAdapter::Adapter::addCheckpointField(volScalarField* field)
{
    if (field)
    {
        volScalarFields_.push_back(field);
        volScalarFieldCopies_.push_back(new volScalarField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(volVectorField* field)
{
    if (field)
    {
        volVectorFields_.push_back(field);
        volVectorFieldCopies_.push_back(new volVectorField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(surfaceScalarField* field)
{
    if (field)
    {
        surfaceScalarFields_.push_back(field);
        surfaceScalarFieldCopies_.push_back(new surfaceScalarField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(surfaceVectorField* field)
{
    if (field)
    {
        surfaceVectorFields_.push_back(field);
        surfaceVectorFieldCopies_.push_back(new surfaceVectorField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(pointScalarField* field)
{
    if (field)
    {
        pointScalarFields_.push_back(field);
        pointScalarFieldCopies_.push_back(new pointScalarField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(pointVectorField* field)
{
    if (field)
    {
        pointVectorFields_.push_back(field);
        pointVectorFieldCopies_.push_back(new pointVectorField(*field));
        // TODO: Old time
        // pointVectorFieldsOld_.push_back(const_cast<pointVectorField&>(field->oldTime())));
        // pointVectorFieldCopiesOld_.push_back(new pointVectorField(field->oldTime()));
    }
}

void preciceAdapter::Adapter::addCheckpointField(volTensorField* field)
{
    if (field)
    {
        volTensorFields_.push_back(field);
        volTensorFieldCopies_.push_back(new volTensorField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(surfaceTensorField* field)
{
    if (field)
    {
        surfaceTensorFields_.push_back(field);
        surfaceTensorFieldCopies_.push_back(new surfaceTensorField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(pointTensorField* field)
{
    if (field)
    {
        pointTensorFields_.push_back(field);
        pointTensorFieldCopies_.push_back(new pointTensorField(*field));
    }
}

void preciceAdapter::Adapter::addCheckpointField(volSymmTensorField* field)
{
    if (field)
    {
        volSymmTensorFields_.push_back(field);
        volSymmTensorFieldCopies_.push_back(new volSymmTensorField(*field));
    }
}


// NOTE: Add here methods to add other object types to checkpoint, if needed.

void preciceAdapter::Adapter::readCheckpoint()
{
    SETUP_TIMER();

    // TODO: To increase efficiency: only the oldTime() fields of the quantities which are used in the time
    //  derivative are necessary. (In general this is only the velocity). Also old information of the mesh
    //  is required.
    //  Therefore, loading the oldTime() and oldTime().oldTime() fields for the other fields can be excluded
    //  for efficiency.
    DEBUG(adapterInfo("Reading a checkpoint..."));

    // Reload the runTime
    reloadCheckpointTime();

    // Reload the meshPoints (if FSI is enabled)
    if (FSIenabled_)
    {
        reloadMeshPoints();
    }

    // Reload all the fields of type volScalarField
    for (uint i = 0; i < volScalarFields_.size(); i++)
    {
        // Load the volume field
        *(volScalarFields_.at(i)) == *(volScalarFieldCopies_.at(i));
        // TODO: Do we need this?
        // *(volScalarFields_.at(i))->boundaryField() = *(volScalarFieldCopies_.at(i))->boundaryField();

        int nOldTimes(volScalarFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            volScalarFields_.at(i)->oldTime() == volScalarFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            volScalarFields_.at(i)->oldTime().oldTime() == volScalarFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type volVectorField
    for (uint i = 0; i < volVectorFields_.size(); i++)
    {
        // Load the volume field
        *(volVectorFields_.at(i)) == *(volVectorFieldCopies_.at(i));

        int nOldTimes(volVectorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            volVectorFields_.at(i)->oldTime() == volVectorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            volVectorFields_.at(i)->oldTime().oldTime() == volVectorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type surfaceScalarField
    for (uint i = 0; i < surfaceScalarFields_.size(); i++)
    {
        *(surfaceScalarFields_.at(i)) == *(surfaceScalarFieldCopies_.at(i));

        int nOldTimes(surfaceScalarFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            surfaceScalarFields_.at(i)->oldTime() == surfaceScalarFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            surfaceScalarFields_.at(i)->oldTime().oldTime() == surfaceScalarFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type surfaceVectorField
    for (uint i = 0; i < surfaceVectorFields_.size(); i++)
    {
        *(surfaceVectorFields_.at(i)) == *(surfaceVectorFieldCopies_.at(i));

        int nOldTimes(surfaceVectorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            surfaceVectorFields_.at(i)->oldTime() == surfaceVectorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            surfaceVectorFields_.at(i)->oldTime().oldTime() == surfaceVectorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type pointScalarField
    for (uint i = 0; i < pointScalarFields_.size(); i++)
    {
        *(pointScalarFields_.at(i)) == *(pointScalarFieldCopies_.at(i));

        int nOldTimes(pointScalarFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            pointScalarFields_.at(i)->oldTime() == pointScalarFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            pointScalarFields_.at(i)->oldTime().oldTime() == pointScalarFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type pointVectorField
    for (uint i = 0; i < pointVectorFields_.size(); i++)
    {
        // Load the volume field
        *(pointVectorFields_.at(i)) == *(pointVectorFieldCopies_.at(i));

        int nOldTimes(pointVectorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            pointVectorFields_.at(i)->oldTime() == pointVectorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            pointVectorFields_.at(i)->oldTime().oldTime() == pointVectorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // TODO Evaluate if all the tensor fields need to be in here.
    // Reload all the fields of type volTensorField
    for (uint i = 0; i < volTensorFields_.size(); i++)
    {
        *(volTensorFields_.at(i)) == *(volTensorFieldCopies_.at(i));

        int nOldTimes(volTensorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            volTensorFields_.at(i)->oldTime() == volTensorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            volTensorFields_.at(i)->oldTime().oldTime() == volTensorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type surfaceTensorField
    for (uint i = 0; i < surfaceTensorFields_.size(); i++)
    {
        *(surfaceTensorFields_.at(i)) == *(surfaceTensorFieldCopies_.at(i));

        int nOldTimes(surfaceTensorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            surfaceTensorFields_.at(i)->oldTime() == surfaceTensorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            surfaceTensorFields_.at(i)->oldTime().oldTime() == surfaceTensorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type pointTensorField
    for (uint i = 0; i < pointTensorFields_.size(); i++)
    {
        *(pointTensorFields_.at(i)) == *(pointTensorFieldCopies_.at(i));

        int nOldTimes(pointTensorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            pointTensorFields_.at(i)->oldTime() == pointTensorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            pointTensorFields_.at(i)->oldTime().oldTime() == pointTensorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // TODO volSymmTensorField is new.
    // Reload all the fields of type volSymmTensorField
    for (uint i = 0; i < volSymmTensorFields_.size(); i++)
    {
        *(volSymmTensorFields_.at(i)) == *(volSymmTensorFieldCopies_.at(i));

        int nOldTimes(volSymmTensorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            volSymmTensorFields_.at(i)->oldTime() == volSymmTensorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            volSymmTensorFields_.at(i)->oldTime().oldTime() == volSymmTensorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // NOTE: Add here other field types to read, if needed.

#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Checkpoint was read. Time = " + std::to_string(runTime_.value()));
#endif

    ACCUMULATE_TIMER(timeInCheckpointingRead_);

    return;
}


void preciceAdapter::Adapter::writeCheckpoint()
{
    SETUP_TIMER();

    DEBUG(adapterInfo("Writing a checkpoint..."));

    // Store the runTime
    storeCheckpointTime();

    // Store the meshPoints (if FSI is enabled)
    if (FSIenabled_)
    {
        storeMeshPoints();
    }

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

    // Store all the fields of type volTensorField
    for (uint i = 0; i < volTensorFields_.size(); i++)
    {
        *(volTensorFieldCopies_.at(i)) == *(volTensorFields_.at(i));
    }

    // Store all the fields of type volSymmTensorField
    for (uint i = 0; i < volSymmTensorFields_.size(); i++)
    {
        *(volSymmTensorFieldCopies_.at(i)) == *(volSymmTensorFields_.at(i));
    }

    // Store all the fields of type surfaceScalarField
    for (uint i = 0; i < surfaceScalarFields_.size(); i++)
    {
        *(surfaceScalarFieldCopies_.at(i)) == *(surfaceScalarFields_.at(i));
    }

    // Store all the fields of type surfaceVectorField
    for (uint i = 0; i < surfaceVectorFields_.size(); i++)
    {
        *(surfaceVectorFieldCopies_.at(i)) == *(surfaceVectorFields_.at(i));
    }

    // Store all the fields of type surfaceTensorField
    for (uint i = 0; i < surfaceTensorFields_.size(); i++)
    {
        *(surfaceTensorFieldCopies_.at(i)) == *(surfaceTensorFields_.at(i));
    }

    // Store all the fields of type pointScalarField
    for (uint i = 0; i < pointScalarFields_.size(); i++)
    {
        *(pointScalarFieldCopies_.at(i)) == *(pointScalarFields_.at(i));
    }

    // Store all the fields of type pointVectorField
    for (uint i = 0; i < pointVectorFields_.size(); i++)
    {
        *(pointVectorFieldCopies_.at(i)) == *(pointVectorFields_.at(i));
    }

    // Store all the fields of type pointTensorField
    for (uint i = 0; i < pointTensorFields_.size(); i++)
    {
        *(pointTensorFieldCopies_.at(i)) == *(pointTensorFields_.at(i));
    }
    // NOTE: Add here other types to write, if needed.

#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Checkpoint for time t = " + std::to_string(runTime_.value()) + " was stored.");
#endif

    ACCUMULATE_TIMER(timeInCheckpointingWrite_);

    return;
}

void preciceAdapter::Adapter::readMeshCheckpoint()
{
    DEBUG(adapterInfo("Reading a mesh checkpoint..."));

    // TODO only the meshPhi field is here, which is a surfaceScalarField. The other fields can be removed.
    //  Reload all the fields of type mesh surfaceScalarField
    for (uint i = 0; i < meshSurfaceScalarFields_.size(); i++)
    {
        // Load the volume field
        *(meshSurfaceScalarFields_.at(i)) == *(meshSurfaceScalarFieldCopies_.at(i));

        int nOldTimes(meshSurfaceScalarFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            meshSurfaceScalarFields_.at(i)->oldTime() == meshSurfaceScalarFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            meshSurfaceScalarFields_.at(i)->oldTime().oldTime() == meshSurfaceScalarFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type mesh surfaceVectorField
    for (uint i = 0; i < meshSurfaceVectorFields_.size(); i++)
    {
        // Load the volume field
        *(meshSurfaceVectorFields_.at(i)) == *(meshSurfaceVectorFieldCopies_.at(i));

        int nOldTimes(meshSurfaceVectorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            meshSurfaceVectorFields_.at(i)->oldTime() == meshSurfaceVectorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            meshSurfaceVectorFields_.at(i)->oldTime().oldTime() == meshSurfaceVectorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

    // Reload all the fields of type mesh volVectorField
    for (uint i = 0; i < meshVolVectorFields_.size(); i++)
    {
        // Load the volume field
        *(meshVolVectorFields_.at(i)) == *(meshVolVectorFieldCopies_.at(i));

        int nOldTimes(meshVolVectorFields_.at(i)->nOldTimes());
        if (nOldTimes >= 1)
        {
            meshVolVectorFields_.at(i)->oldTime() == meshVolVectorFieldCopies_.at(i)->oldTime();
        }
        if (nOldTimes == 2)
        {
            meshVolVectorFields_.at(i)->oldTime().oldTime() == meshVolVectorFieldCopies_.at(i)->oldTime().oldTime();
        }
    }

#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Mesh checkpoint was read. Time = " + std::to_string(runTime_.value()));
#endif

    return;
}

void preciceAdapter::Adapter::writeMeshCheckpoint()
{
    DEBUG(adapterInfo("Writing a mesh checkpoint..."));

    // Store all the fields of type mesh surfaceScalar
    for (uint i = 0; i < meshSurfaceScalarFields_.size(); i++)
    {
        *(meshSurfaceScalarFieldCopies_.at(i)) == *(meshSurfaceScalarFields_.at(i));
    }

    // Store all the fields of type mesh surfaceVector
    for (uint i = 0; i < meshSurfaceVectorFields_.size(); i++)
    {
        *(meshSurfaceVectorFieldCopies_.at(i)) == *(meshSurfaceVectorFields_.at(i));
    }

    // Store all the fields of type mesh volVector
    for (uint i = 0; i < meshVolVectorFields_.size(); i++)
    {
        *(meshVolVectorFieldCopies_.at(i)) == *(meshVolVectorFields_.at(i));
    }

#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Mesh checkpoint for time t = " + std::to_string(runTime_.value()) + " was stored.");
#endif

    return;
}

// TODO for the volumes of the mesh, check this part for subcycling.
void preciceAdapter::Adapter::readVolCheckpoint()
{
    DEBUG(adapterInfo("Reading the mesh volumes checkpoint..."));

    // Reload all the fields of type mesh volVectorField::Internal
    for (uint i = 0; i < volScalarInternalFields_.size(); i++)
    {
        // Load the volume field
        *(volScalarInternalFields_.at(i)) = *(volScalarInternalFieldCopies_.at(i));
        // There are no old times for the internal fields.
    }

#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Mesh volumes were read. Time = " + std::to_string(runTime_.value()));
#endif

    return;
}

void preciceAdapter::Adapter::writeVolCheckpoint()
{
    DEBUG(adapterInfo("Writing a mesh volumes checkpoint..."));

    // Store all the fields of type mesh volScalarField::Internal
    for (uint i = 0; i < volScalarInternalFields_.size(); i++)
    {
        *(volScalarInternalFieldCopies_.at(i)) = *(volScalarInternalFields_.at(i));
    }

#ifdef ADAPTER_DEBUG_MODE
    adapterInfo(
        "Mesh volumes checkpoint for time t = " + std::to_string(runTime_.value()) + " was stored.");
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

        // Fields
        // volScalarFields
        for (uint i = 0; i < volScalarFieldCopies_.size(); i++)
        {
            delete volScalarFieldCopies_.at(i);
        }
        volScalarFieldCopies_.clear();
        // volVector
        for (uint i = 0; i < volVectorFieldCopies_.size(); i++)
        {
            delete volVectorFieldCopies_.at(i);
        }
        volVectorFieldCopies_.clear();
        // surfaceScalar
        for (uint i = 0; i < surfaceScalarFieldCopies_.size(); i++)
        {
            delete surfaceScalarFieldCopies_.at(i);
        }
        surfaceScalarFieldCopies_.clear();
        // surfaceVector
        for (uint i = 0; i < surfaceVectorFieldCopies_.size(); i++)
        {
            delete surfaceVectorFieldCopies_.at(i);
        }
        surfaceVectorFieldCopies_.clear();
        // pointScalar
        for (uint i = 0; i < pointScalarFieldCopies_.size(); i++)
        {
            delete pointScalarFieldCopies_.at(i);
        }
        pointScalarFieldCopies_.clear();
        // pointVector
        for (uint i = 0; i < pointVectorFieldCopies_.size(); i++)
        {
            delete pointVectorFieldCopies_.at(i);
        }
        pointVectorFieldCopies_.clear();

        // Mesh fields
        // meshSurfaceScalar
        for (uint i = 0; i < meshSurfaceScalarFieldCopies_.size(); i++)
        {
            delete meshSurfaceScalarFieldCopies_.at(i);
        }
        meshSurfaceScalarFieldCopies_.clear();

        // meshSurfaceVector
        for (uint i = 0; i < meshSurfaceVectorFieldCopies_.size(); i++)
        {
            delete meshSurfaceVectorFieldCopies_.at(i);
        }
        meshSurfaceVectorFieldCopies_.clear();

        // meshVolVector
        for (uint i = 0; i < meshVolVectorFieldCopies_.size(); i++)
        {
            delete meshVolVectorFieldCopies_.at(i);
        }
        meshVolVectorFieldCopies_.clear();

        // TODO for the internal volume
        //  volScalarInternal
        for (uint i = 0; i < volScalarInternalFieldCopies_.size(); i++)
        {
            delete volScalarInternalFieldCopies_.at(i);
        }
        volScalarInternalFieldCopies_.clear();

        // volTensorField
        for (uint i = 0; i < volTensorFieldCopies_.size(); i++)
        {
            delete volTensorFieldCopies_.at(i);
        }
        volTensorFieldCopies_.clear();

        // surfaceTensorField
        for (uint i = 0; i < surfaceTensorFieldCopies_.size(); i++)
        {
            delete surfaceTensorFieldCopies_.at(i);
        }
        surfaceTensorFieldCopies_.clear();

        // pointTensorField
        for (uint i = 0; i < pointTensorFieldCopies_.size(); i++)
        {
            delete pointTensorFieldCopies_.at(i);
        }
        pointTensorFieldCopies_.clear();

        // volSymmTensor
        for (uint i = 0; i < volSymmTensorFieldCopies_.size(); i++)
        {
            delete volSymmTensorFieldCopies_.at(i);
        }
        volSymmTensorFieldCopies_.clear();

        // NOTE: Add here delete for other types, if needed

        checkpointing_ = false;
    }

    // Delete the CHT module
    if (NULL != CHT_)
    {
        DEBUG(adapterInfo("Destroying the CHT module..."));
        delete CHT_;
        CHT_ = NULL;
    }

    // Delete the FSI module
    if (NULL != FSI_)
    {
        DEBUG(adapterInfo("Destroying the FSI module..."));
        delete FSI_;
        FSI_ = NULL;
    }

    // Delete the FF module
    if (NULL != FF_)
    {
        DEBUG(adapterInfo("Destroying the FF module..."));
        delete FF_;
        FF_ = NULL;
    }

    // NOTE: Delete your new module here

    return;
}

preciceAdapter::Adapter::~Adapter()
{
    teardown();

    TIMING_MODE(
        // Continuing the output started in the destructor of preciceAdapterFunctionObject
        Info << "Time exclusively in the adapter: " << (timeInConfigRead_ + timeInMeshSetup_ + timeInCheckpointingSetup_ + timeInWrite_ + timeInRead_ + timeInCheckpointingWrite_ + timeInCheckpointingRead_).str() << nl;
        Info << "  (S) reading preciceDict:       " << timeInConfigRead_.str() << nl;
        Info << "  (S) constructing preCICE:      " << timeInPreciceConstruct_.str() << nl;
        Info << "  (S) setting up the interfaces: " << timeInMeshSetup_.str() << nl;
        Info << "  (S) setting up checkpointing:  " << timeInCheckpointingSetup_.str() << nl;
        Info << "  (I) writing data:              " << timeInWrite_.str() << nl;
        Info << "  (I) reading data:              " << timeInRead_.str() << nl;
        Info << "  (I) writing checkpoints:       " << timeInCheckpointingWrite_.str() << nl;
        Info << "  (I) reading checkpoints:       " << timeInCheckpointingRead_.str() << nl;
        Info << "  (I) writing OpenFOAM results:  " << timeInWriteResults_.str() << " (at the end of converged time windows)" << nl << nl;
        Info << "Time exclusively in preCICE:     " << (timeInInitialize_ + timeInAdvance_ + timeInFinalize_).str() << nl;
        Info << "  (S) initialize():              " << timeInInitialize_.str() << nl;
        Info << "  (I) advance():                 " << timeInAdvance_.str() << nl;
        Info << "  (I) finalize():                " << timeInFinalize_.str() << nl;
        Info << "  These times include time waiting for other participants." << nl;
        Info << "  See also precice-<participant>-events-summary.log." << nl;
        Info << "-------------------------------------------------------------------------------------" << nl;)

    return;
}
