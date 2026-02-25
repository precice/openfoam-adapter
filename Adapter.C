#include "Adapter.H"
#include "Interface.H"
#include "Utilities.H"

#include "IOstreams.H"
#include <algorithm>

using namespace Foam;

preciceAdapter::Adapter::Adapter(const Time& runTime, const fvMesh& mesh)
: runTime_(runTime),
  mesh_(mesh)
{
    adapterInfo("Loaded the OpenFOAM-preCICE adapter - v1.3.1.", "info");

    return;
}

void preciceAdapter::Adapter::readFieldConfigs(const std::string& listName, Foam::ITstream& stream, std::vector<FieldConfig>& configs)
{
    // Perform check on whether read/writeData is a list
    if (stream.peek() == token::BEGIN_LIST)
    {
        token _t;
        stream >> _t; // First token is '(', which we throw away

        // Read stream until end of list
        while (stream.peek() != token::END_LIST && !stream.eof())
        {
            // Next token is always a word (the data name)
            word dataName;
            stream >> dataName;

            struct FieldConfig fieldConfig;

            // If next token is '{', we have a dictionary (new schema).
            // We create a dictionary from the stream `dictionary dict(stream);`
            // The dictionary must contain the 'name'.
            // If 'solver_name' is not specified, it defaults to the same as 'name'.
            // 'operation' defaults to 'value'.
            // Note: Currently, the modules FF, CHT, and FSI do not use solver_name/operation.
            if (stream.peek() == token::BEGIN_BLOCK)
            {
                dictionary dict(stream);
                fieldConfig.name = dict.get<word>("name"); // The 'name' entry is mandatory.
                fieldConfig.solver_name = dict.lookupOrDefault<word>("solver_name", fieldConfig.name);
                fieldConfig.operation = dict.lookupOrDefault<word>("operation", "value");
                try
                {
                    fieldConfig.flip_normal = dict.lookupOrDefault<bool>("flip-normal", false);
                }
                catch (const Foam::IOerror& e)
                {
                    adapterInfo("Error parsing 'flip-normal' for field " + dataName + "\n" + e.message(), "error");
                }
            }
            // Else, we have a simple word entry (legacy schema/backwards compatibility).
            else
            {
                fieldConfig.name = dataName;
                fieldConfig.solver_name = "Undefined (legacy mode)";
                fieldConfig.operation = "Undefined (legacy mode)";
                fieldConfig.flip_normal = false;
            }

            configs.push_back(fieldConfig);

            DEBUG(adapterInfo("      - " + dataName));
            DEBUG(adapterInfo("        name: " + fieldConfig.name));
            DEBUG(adapterInfo("        solver_name: " + fieldConfig.solver_name));
            DEBUG(adapterInfo("        operation  : " + fieldConfig.operation));
            DEBUG(adapterInfo("        flip-normal: " + std::string(fieldConfig.flip_normal ? "true" : "false")));
        }
        stream >> _t; // Last token ')'
    }
    else
    {
        adapterInfo(listName + " must be a list", "error");
    }
}

void preciceAdapter::Adapter::configFileRead()
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

        if (module == "generic")
        {
            genericModuleEnabled_ = true;
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
        adapterInfo("  Empty list of interfaces", "error");
        return;
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
                if (interfaceConfig.meshConnectivity && (interfaceConfig.locationsType == "faceCenters" || interfaceConfig.locationsType == "volumeCenters" || interfaceConfig.locationsType == "volumeCentres"))
                {
                    DEBUG(adapterInfo("Mesh connectivity is not supported for faceCenters or volumeCenters. \n"
                                      "Please configure the desired interface with the locationsType faceNodes. \n"
                                      "Have a look in the adapter documentation for detailed information.",
                                      "error"));
                    return;
                }
                DEBUG(adapterInfo("    connectivity : " + std::to_string(interfaceConfig.meshConnectivity)));

                DEBUG(adapterInfo("    patches      : "));
                auto patches = interfaceDict.get<wordList>("patches");
                for (auto patch : patches)
                {
                    interfaceConfig.patchNames.push_back(patch);
                    DEBUG(adapterInfo("      - " + patch));
                }

                DEBUG(adapterInfo("    cellSets      : "));
                auto cellSets = interfaceDict.lookupOrDefault<wordList>("cellSets", wordList());

                for (auto cellSet : cellSets)
                {
                    interfaceConfig.cellSetNames.push_back(cellSet);
                    DEBUG(adapterInfo("      - " + cellSet));
                }

                if (!interfaceConfig.cellSetNames.empty() && !(interfaceConfig.locationsType == "volumeCenters" || interfaceConfig.locationsType == "volumeCentres"))
                {
                    adapterInfo("Cell sets are not supported for locationType != volumeCenters. \n"
                                "Please configure the desired interface with the locationsType volumeCenters. \n"
                                "Have a look in the adapter documentation for detailed information.",
                                "error");
                    return;
                }

                if (interfaceDict.found("writeData"))
                {
                    DEBUG(adapterInfo("    writeData    : "));
                    ITstream writeDataStream = interfaceDict.lookup("writeData");
                    readFieldConfigs("writeData", writeDataStream, interfaceConfig.writeData);
                }

                if (interfaceDict.found("readData"))
                {
                    DEBUG(adapterInfo("    readData     : "));
                    ITstream readDataStream = interfaceDict.lookup("readData");
                    readFieldConfigs("readData", readDataStream, interfaceConfig.readData);
                }

                interfacesConfig_.push_back(interfaceConfig);
            }
        }
    }

    // NOTE: set the switch for your new module here

    if (genericModuleEnabled_)
    {
        Generic_ = new Generic::GenericInterface(mesh_);
        if (!Generic_->configure(preciceDict))
        {
            return;
        }
    }

    // If the CHT module is enabled, create it, read the
    // CHT-specific options and configure it.
    if (CHTenabled_)
    {
        CHT_ = new CHT::ConjugateHeatTransfer(mesh_);
        if (!CHT_->configure(preciceDict))
        {
            adapterInfo("There was an error while configuring the CHT module",
                        "error");
            return;
        }
    }

    // If the FSI module is enabled, create it, read the
    // FSI-specific options and configure it.
    if (FSIenabled_)
    {
        FSI_ = new FSI::FluidStructureInteraction(mesh_, runTime_);
        if (!FSI_->configure(preciceDict))
        {
            adapterInfo("There was an error while configuring the FSI module",
                        "error");
            return;
        }
    }

    if (FFenabled_)
    {
        FF_ = new FF::FluidFluid(mesh_);
        if (!FF_->configure(preciceDict))
        {
            adapterInfo("There was an error while configuring the FF module",
                        "error");
            return;
        }
    }

    // NOTE: Create your module and read any options specific to it here

    if (!CHTenabled_ && !FSIenabled_ && !FFenabled_ && !genericModuleEnabled_) // NOTE: Add your new switch here
    {
        adapterInfo("No module is enabled.", "error");
        return;
    }

    // TODO: Loading modules should be implemented in more general way,
    // in order to avoid code duplication. See issue #16 on GitHub.

    ACCUMULATE_TIMER(timeInConfigRead_);

    return;
}

void preciceAdapter::Adapter::configure()
try
{
    // Read the adapter's configuration file
    configFileRead();

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

        Interface* interface = new Interface(*precice_, mesh_, interfacesConfig_.at(i).meshName, interfacesConfig_.at(i).locationsType, interfacesConfig_.at(i).patchNames, interfacesConfig_.at(i).cellSetNames, interfacesConfig_.at(i).meshConnectivity, restartFromDeformed, namePointDisplacement, nameCellDisplacement);
        interfaces_.push_back(interface);
        DEBUG(adapterInfo("Interface created on mesh " + interfacesConfig_.at(i).meshName));

        DEBUG(adapterInfo("Adding coupling data writers..."));
        for (uint j = 0; j < interfacesConfig_.at(i).writeData.size(); j++)
        {
            FieldConfig fieldConfig = interfacesConfig_.at(i).writeData.at(j);
            std::string dataName = fieldConfig.name;

            unsigned int inModules = 0;

            // Add CHT-related coupling data writers
            if (CHTenabled_ && CHT_->addWriters(fieldConfig, interface))
            {
                inModules++;
            }

            // Add FSI-related coupling data writers
            if (FSIenabled_ && FSI_->addWriters(fieldConfig, interface))
            {
                inModules++;
            }

            // Add FF-related coupling data writers
            if (FFenabled_ && FF_->addWriters(fieldConfig, interface))
            {
                inModules++;
            }

            // Add generic module coupling data writers
            // Only add Generic interface if not found in other modules
            if (inModules == 0)
            {
                if (genericModuleEnabled_ && Generic_->addWriters(fieldConfig, interface))
                {
                    inModules++;
                };
            }

            if (inModules == 0)
            {
                adapterInfo("I don't know how to write \"" + dataName
                                + "\". Maybe this is a typo or maybe you need to enable some adapter module?",
                            "error");
            }
            else if (inModules > 1)
            {
                adapterInfo("It looks like more than one modules can write \"" + dataName
                                + "\" and I don't know how to choose. Try disabling one of the modules.",
                            "error");
            }

            // NOTE: Add any coupling data writers for your module here.
        } // end add coupling data writers

        DEBUG(adapterInfo("Adding coupling data readers..."));
        for (uint j = 0; j < interfacesConfig_.at(i).readData.size(); j++)
        {
            FieldConfig fieldConfig = interfacesConfig_.at(i).readData.at(j);
            std::string dataName = fieldConfig.name;

            unsigned int inModules = 0;

            // Add CHT-related coupling data readers
            if (CHTenabled_ && CHT_->addReaders(fieldConfig, interface))
            {
                inModules++;
            }

            // Add FSI-related coupling data readers
            if (FSIenabled_ && FSI_->addReaders(fieldConfig, interface))
            {
                inModules++;
            }

            // Add FF-related coupling data readers
            if (FFenabled_ && FF_->addReaders(fieldConfig, interface))
            {
                inModules++;
            }

            // Add generic module coupling data readers
            // Only add Generic interface if not found in other modules
            if (inModules == 0)
            {
                if (genericModuleEnabled_ && Generic_->addReaders(fieldConfig, interface))
                {
                    inModules++;
                }
            }

            if (inModules == 0)
            {
                adapterInfo("I don't know how to read \"" + dataName
                                + "\". Maybe this is a typo or maybe you need to enable some adapter module?",
                            "error");
            }
            else if (inModules > 1)
            {
                adapterInfo("It looks like more than one modules can read \"" + dataName
                                + "\" and I don't know how to choose. Try disabling one of the modules.",
                            "error");
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

    return;
}
catch (const PreciceError& e)
{
    std::exit(EXIT_FAILURE);
}

void preciceAdapter::Adapter::execute()
try
{

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
        pruneCheckpointedFields();
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
catch (const PreciceError& e)
{
    std::exit(EXIT_FAILURE);
}


void preciceAdapter::Adapter::adjustTimeStep()
try
{
    adjustSolverTimeStepAndReadData();

    return;
}
catch (const PreciceError& e)
{
    std::exit(EXIT_FAILURE);
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
    {
        DEBUG(adapterInfo("Initializing preCICE data..."));
        writeCouplingData();
    }

    precice_->initialize();
    preciceInitialized_ = true;
    ACCUMULATE_TIMER(timeInInitialize_);

    adapterInfo("preCICE was configured and initialized", "info");

    return;
}

void preciceAdapter::Adapter::finalize()
{
    if (nullptr != precice_ && preciceInitialized_ && !isCouplingOngoing())
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
    double tolerance = 1e-14;
    if (precice_->getMaxTimeStepSize() - timestepSolverDetermined > tolerance)
    {
        adapterInfo(
            "The solver's timestep is smaller than the "
            "coupling timestep. Subcycling...",
            "info");
        timestepSolver_ = timestepSolverDetermined;
        if (FSIenabled_)
        {
            adapterInfo(
                "The adapter does not fully support subcycling for FSI and instabilities may occur.",
                "warning");
        }
    }
    else if (timestepSolverDetermined - precice_->getMaxTimeStepSize() > tolerance)
    {
        // In the last time-step, we adjust to dt = 0, but we don't need to trigger the warning here
        if (isCouplingOngoing())
        {
            adapterInfo(
                "The solver's timestep cannot be larger than the coupling timestep."
                " Adjusting from "
                    + std::to_string(timestepSolverDetermined) + " to " + std::to_string(precice_->getMaxTimeStepSize()),
                "warning");
        }
        timestepSolver_ = precice_->getMaxTimeStepSize();
    }
    else
    {
        DEBUG(adapterInfo("The solver's timestep is the same as the "
                          "coupling timestep."));
        timestepSolver_ = precice_->getMaxTimeStepSize();
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
    if (nullptr != precice_)
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
    if (!meshPoints_)
    {
        DEBUG(adapterInfo("Storing mesh points..."));
        // Add points and oldPoints
        meshPoints_ = new Foam::pointField(mesh_.points());
        meshOldPoints_ = new Foam::pointField(mesh_.oldPoints());
    }

    if (mesh_.moving())
    {
        if (!meshCheckPointed_)
        {
            // Set up the checkpoint for the mesh flux: meshPhi
            setupMeshCheckpointing();
            meshCheckPointed_ = true;
        }
        writeMeshCheckpoint();
    }
}

void preciceAdapter::Adapter::reloadMeshPoints()
{
    if (!mesh_.moving())
    {
        DEBUG(adapterInfo("Mesh points not moved as the mesh is not moving"));
        return;
    }

    // Reload mesh points
    const_cast<Foam::fvMesh&>(mesh_).movePoints(*meshPoints_);

    // polyMesh.movePoints will only update oldPoints
    // if (curMotionTimeIndex_ != time().timeIndex())
    const_cast<pointField&>(mesh_.oldPoints()) = *meshOldPoints_;

    readMeshCheckpoint();

    DEBUG(adapterInfo("Moved mesh points to their previous locations."));
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
    // Add meshPhi (Face motion flux)
    addMeshCheckpointField(const_cast<surfaceScalarField&>(mesh_.phi()));

    DEBUG(adapterInfo("Added " + mesh_.phi().name() + " to the list of checkpointed fields."));
}


void preciceAdapter::Adapter::setupCheckpointing()
{
    SETUP_TIMER();

    // Add fields in the checkpointing list - sorted for parallel consistency
    DEBUG(adapterInfo("Adding in checkpointed fields..."));

#undef doLocalCode
#define doLocalCode(GeomFieldType)                                           \
    /* Checkpoint registered GeomFieldType objects */                        \
    for (const word& obj : mesh_.sortedNames<GeomFieldType>())               \
    {                                                                        \
        addCheckpointField(mesh_.thisDb().getObjectPtr<GeomFieldType>(obj)); \
        DEBUG(adapterInfo("Checkpoint " + obj + " : " #GeomFieldType));      \
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

void preciceAdapter::Adapter::pruneCheckpointedFields()
{
    // Check if checkpointed fields exist in OpenFOAM registry
    // If not, remove them from the checkpointed fields vector

    word fieldName;
    uint index;
    std::vector<word> regFields;
    std::vector<uint> toRemoveIndices;

#undef doLocalCode
#define doLocalCode(GeomFieldType, GeomField_, GeomFieldCopies_)                                                                              \
    regFields.clear();                                                                                                                        \
    toRemoveIndices.clear();                                                                                                                  \
    index = 0;                                                                                                                                \
    /* Iterate through fields in OpenFOAM registry */                                                                                         \
    for (const word& fieldName : mesh_.sortedNames<GeomFieldType>())                                                                          \
    {                                                                                                                                         \
        regFields.push_back(fieldName);                                                                                                       \
    }                                                                                                                                         \
    /* Iterate through checkpointed fields */                                                                                                 \
    for (GeomFieldType * fieldObj : GeomFieldCopies_)                                                                                         \
    {                                                                                                                                         \
        fieldName = fieldObj->name();                                                                                                         \
        if (std::find(regFields.begin(), regFields.end(), fieldName) == regFields.end())                                                      \
        {                                                                                                                                     \
            toRemoveIndices.push_back(index);                                                                                                 \
        }                                                                                                                                     \
        index += 1;                                                                                                                           \
    }                                                                                                                                         \
    if (!toRemoveIndices.empty())                                                                                                             \
    {                                                                                                                                         \
        /* Iterate in reverse to avoid index shifting */                                                                                      \
        for (auto it = toRemoveIndices.rbegin(); it != toRemoveIndices.rend(); ++it)                                                          \
        {                                                                                                                                     \
            index = *it;                                                                                                                      \
            DEBUG(adapterInfo("Removed " #GeomFieldType " : " + GeomFieldCopies_.at(index)->name() + " from the checkpointed fields list.")); \
            GeomField_.erase(GeomField_.begin() + index);                                                                                     \
            delete GeomFieldCopies_.at(index);                                                                                                \
            GeomFieldCopies_.erase(GeomFieldCopies_.begin() + index);                                                                         \
        }                                                                                                                                     \
    }

    doLocalCode(volScalarField, volScalarFields_, volScalarFieldCopies_);
    doLocalCode(volVectorField, volVectorFields_, volVectorFieldCopies_);
    doLocalCode(volTensorField, volTensorFields_, volTensorFieldCopies_);
    doLocalCode(volSymmTensorField, volSymmTensorFields_, volSymmTensorFieldCopies_);

    doLocalCode(surfaceScalarField, surfaceScalarFields_, surfaceScalarFieldCopies_);
    doLocalCode(surfaceVectorField, surfaceVectorFields_, surfaceVectorFieldCopies_);
    doLocalCode(surfaceTensorField, surfaceTensorFields_, surfaceTensorFieldCopies_);

    doLocalCode(pointScalarField, pointScalarFields_, pointScalarFieldCopies_);
    doLocalCode(pointVectorField, pointVectorFields_, pointVectorFieldCopies_);
    doLocalCode(pointTensorField, pointTensorFields_, pointTensorFieldCopies_);

#undef doLocalCode
}

// All mesh checkpointed fields

void preciceAdapter::Adapter::addMeshCheckpointField(surfaceScalarField& field)
{
    meshSurfaceScalarFields_.push_back(&field);
    meshSurfaceScalarFieldCopies_.push_back(new surfaceScalarField(field));
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

    DEBUG(adapterInfo("Checkpoint was read. Time = " + std::to_string(runTime_.value())));

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

    DEBUG(adapterInfo("Checkpoint for time t = " + std::to_string(runTime_.value()) + " was stored."));

    ACCUMULATE_TIMER(timeInCheckpointingWrite_);

    return;
}

void preciceAdapter::Adapter::readMeshCheckpoint()
{
    DEBUG(adapterInfo("Reading a mesh checkpoint..."));

    // Only the meshPhi field is here, which is a surfaceScalarField.
    for (uint i = 0; i < meshSurfaceScalarFields_.size(); i++)
    {
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

    DEBUG(adapterInfo("Mesh checkpoint was read. Time = " + std::to_string(runTime_.value())));

    return;
}

void preciceAdapter::Adapter::writeMeshCheckpoint()
{
    DEBUG(adapterInfo("Writing a mesh checkpoint..."));

    // Store all the fields of type mesh surfaceScalar (phi)
    for (uint i = 0; i < meshSurfaceScalarFields_.size(); i++)
    {
        *(meshSurfaceScalarFieldCopies_.at(i)) == *(meshSurfaceScalarFields_.at(i));
    }

    DEBUG(adapterInfo("Storing mesh points..."));

    // Store mesh points
    // swap pointers
    *(meshOldPoints_) = *(meshPoints_);
    *(meshPoints_) = mesh_.points();

    DEBUG(adapterInfo("Mesh checkpoint for time t = " + std::to_string(runTime_.value()) + " was stored."));

    return;
}

void preciceAdapter::Adapter::end()
try
{
    // Throw a warning if the simulation exited before the coupling was complete
    if (nullptr != precice_ && isCouplingOngoing())
    {
        adapterInfo("The solver exited before the coupling was complete.", "warning");
    }

    return;
}
catch (const PreciceError& e)
{
    std::exit(EXIT_FAILURE);
}

void preciceAdapter::Adapter::teardown()
{
    // If the solver interface was not deleted before, delete it now.
    // Normally it should be deleted when isCouplingOngoing() becomes false.
    if (nullptr != precice_)
    {
        DEBUG(adapterInfo("Destroying the preCICE solver interface..."));
        delete precice_;
        precice_ = nullptr;
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

        delete meshPoints_;
        delete meshOldPoints_;
    }

    // Delete the CHT module
    if (nullptr != CHT_)
    {
        DEBUG(adapterInfo("Destroying the CHT module..."));
        delete CHT_;
        CHT_ = nullptr;
    }

    // Delete the FSI module
    if (nullptr != FSI_)
    {
        DEBUG(adapterInfo("Destroying the FSI module..."));
        delete FSI_;
        FSI_ = nullptr;
    }

    // Delete the FF module
    if (nullptr != FF_)
    {
        DEBUG(adapterInfo("Destroying the FF module..."));
        delete FF_;
        FF_ = nullptr;
    }

    // Delete the Generic module
    if (nullptr != Generic_)
    {
        DEBUG(adapterInfo("Destroying the Generic module..."));
        delete Generic_;
        Generic_ = nullptr;
    }

    // NOTE: Delete your new module here

    return;
}

preciceAdapter::Adapter::~Adapter()
try
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
        Info << "  See also precice-profiling on the website https://precice.org/tooling-performance-analysis.html." << nl;
        Info << "-------------------------------------------------------------------------------------" << nl;)

    return;
}
catch (const PreciceError& e)
{
    std::exit(EXIT_FAILURE);
}
