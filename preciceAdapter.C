/*---------------------------------------------------------------------------*\
preCICE-adapter for OpenFOAM

Copyright (c) 2017 Gerasimos Chourdakis
-------------------------------------------------------------------------------

License
    This adapter is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This adapter is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with the adapter.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "preciceAdapter.H"

// OpenFOAM header files
#include "Time.H"
#include "fvMesh.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(preciceAdapter, 0);
    addToRunTimeSelectionTable(functionObject, preciceAdapter, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapter::preciceAdapter
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict),
    runTime_(runTime)
{
    Info << "---[preciceAdapter] CONSTRUCTOR --------" << nl;
    read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::preciceAdapter::~preciceAdapter()
{
    Info << "---[preciceAdapter] DESTRUCTOR ---------" << nl;

    Info << "---[preciceAdapter] Destroy the preCICE Solver Interface." << nl;
    delete precice_;

}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::preciceAdapter::read(const dictionary& dict)
{
    Info << "---[preciceAdapter] READ ---------------" << nl;

    // Read the adapter's YAML configuration file
    configFileRead();


    // Add fields in the checkpointing list
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter] Add checkpoint fields (decide which)." << nl;
    }

    // Check the timestep type (fixed vs adjustable)
    Info << "---[preciceAdapter] Check the timestep type (fixed vs adjustable)." << nl;
    adjustableTimestep_ = runTime_.controlDict().lookupOrDefault("adjustTimeStep", false);

    if (adjustableTimestep_) {
        Info << "---[preciceAdapter]   Timestep type: adjustable." << nl;
    } else {
        Info << "---[preciceAdapter]   Timestep type: fixed." << nl;
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

    Info << "---[preciceAdapter] Write coupling data (for the first iteration)" << nl;
    Info << "---[preciceAdapter] Initialize preCICE data." << nl;
    Info << "---[preciceAdapter] ---" << nl;
    Info << "---[preciceAdapter] Read coupling data (for the first iteration)" << nl;

    // Write checkpoint (for the first iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter] Write checkpoint (for the first iteration)" << nl;
    }

    // Adjust the timestep for the first iteration, if it is fixed
    if (!adjustableTimestep_) {
        Info << "---[preciceAdapter] Adjust the solver's timestep (if fixed timestep, for the first iteration)" << nl;
    }

    return true;
}


bool Foam::functionObjects::preciceAdapter::execute()
{
    Info << "---[preciceAdapter] EXECUTE (i > 1)------" << nl;
    Info << "---[preciceAdapter] if (coupling ongoing) {" << nl;
    Info << "---[preciceAdapter]   Write coupling data (from the previous iteration)." << nl;
    Info << "---[preciceAdapter]   Advance preCICE (from the previous iteration)." << nl;
    Info << "---[preciceAdapter]   Read coupling data (from the previous iteration)." << nl;

    // Adjust the timestep, if it is fixed
    if (!adjustableTimestep_) {
        Info << "---[preciceAdapter]   Adjust the solver's timestep (if fixed timestep, from the previous iteration)." << nl;
    }

    // Read checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter]   Read checkpoint (from the previous iteration)." << nl;
    }

    // Write checkpoint (from the previous iteration)
    if (checkpointingEnabled_) {
        Info << "---[preciceAdapter]   Write checkpoint (from the previous iteration)." << nl;
    }

    Info << "---[preciceAdapter]   Write if coupling timestep complete (?)." << nl;
    Info << "---[preciceAdapter] } else {" << nl;
    Info << "---[preciceAdapter]   Exit the loop." << nl;
    Info << "---[preciceAdapter] }" << nl;
    return true;
}


bool Foam::functionObjects::preciceAdapter::end()
{
    Info << "---[preciceAdapter] END ----------------" << nl;
    return true;
}


bool Foam::functionObjects::preciceAdapter::write()
{
    Info << "---[preciceAdapter] WRITE --------------" << nl;
    return true;
}

bool Foam::functionObjects::preciceAdapter::adjustTimeStep()
{
    Info << "---[preciceAdapter] ADJUSTTIMESTEP -----" << nl;
    Info << "---[preciceAdapter] Adjust the solver's timestep (only if dynamic timestep is used)." << nl;
    return true;
}

void Foam::functionObjects::preciceAdapter::errorAndExit(const std::string message, const std::string function)
{
    // TODO not exiting: stops the function object, but not the simulation.
    // Using the stopAt() it stops at the end of the timestep, but without an error.
    runTime_.stopAt(Time::saNoWriteNow);
    FatalErrorIn(function) << "Error in preciceAdapter:" << nl << message << nl << exit(FatalError);
}

void Foam::functionObjects::preciceAdapter::configFileCheck(const std::string adapterConfigFileName)
{
    Info << "---[preciceAdapter] Check the adapter's YAML configuration file." << nl;

    bool configErrors = false;

    YAML::Node adapterConfig = YAML::LoadFile(adapterConfigFileName);

    // TODO Consider simplifying
    // Check if the "participant" node exists
    if ( !adapterConfig["participant"] )
    {
        Info << "---[preciceAdapter] Error: Adapter's configuration file: participant node is missing." << nl;
        configErrors = true;
    }

    // Check if the "precice-config-file" node exists
    if ( !adapterConfig["precice-config-file"] )
    {
        Info << "---[preciceAdapter] Error: Adapter's configuration file: 'precice-config-file' node is missing." << nl;
        configErrors = true;
        // TODO Check if the specified file exists
    }

    // Check if the "interfaces" node exists
    if ( !adapterConfig["interfaces"] )
    {
        Info << "---[preciceAdapter] Error: Adapter's configuration file: 'interfaces' node is missing." << nl;
        configErrors = true;
    } else {
        for ( uint i = 0; i < adapterConfig["interfaces"].size(); i++ )
        {
            if ( !adapterConfig["interfaces"][i]["mesh"] )
            {
                Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'mesh' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["patches"] )
            {
                Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'patches' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["write-data"] )
            {
                Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'write-data' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
                // TODO Add check for allowed values.
            }
            if ( !adapterConfig["interfaces"][i]["read-data"] )
            {
                Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'read-data' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
                // TODO Add check for allowed values.
            }
        }
    }

    // Check if the "subcycling" node exists
    if ( !adapterConfig["subcycling"] )
    {
        Info << "---[preciceAdapter] Error: Adapter's configuration file: 'subcycling' node is missing." << nl;
        configErrors = true;
    }

    // Check if the "checkpointing" node exists
    if ( !adapterConfig["checkpointing"] )
    {
        Info << "---[preciceAdapter] Error: Adapter's configuration file: 'checkpointing' node is missing." << nl;
        configErrors = true;
    }

    if ( configErrors )
    {
        errorAndExit("The adapter's configuration file " + adapterConfigFileName + " is incomplete or wrong. See the log for details.", "preciceAdapter::configFileCheck()");
    }

    Info << "---[preciceAdapter]   The adapter's YAML configuration file " << adapterConfigFileName << " is complete." << nl;
}

void Foam::functionObjects::preciceAdapter::configFileRead()
{
    Info << "---[preciceAdapter] Read the adapter's YAML configuration file (one per solver)." << nl;

    // Check the configuration file
    const std::string adapterConfigFileName = "precice-adapter-config.yml";
    configFileCheck(adapterConfigFileName);

    // Load the YAML file
    adapterConfig_ = YAML::LoadFile(adapterConfigFileName);

    // Read the preCICE participant name
    participantName_ = adapterConfig_["participant"].as<std::string>();
    Info << "---[preciceAdapter]   participant : " << participantName_ << nl;

    // Read the preCICE configuration file name
    preciceConfigFilename_ = adapterConfig_["precice-config-file"].as<std::string>();
    Info << "---[preciceAdapter]   precice-config-file : " << preciceConfigFilename_ << nl;

    // TODO Read the coupling interfaces
    YAML::Node adapterConfigInterfaces = adapterConfig_["interfaces"];
    Info << "---[preciceAdapter]   interfaces : TODO " << nl;

    // Set the subcyclingAllowed_ switch
    subcyclingAllowed_ = adapterConfig_["subcycling"].as<bool>();
    Info << "---[preciceAdapter]   subcycling : " << subcyclingAllowed_ << nl;

    // Set the checkpointingEnabled_ switch
    checkpointingEnabled_ = adapterConfig_["checkpointing"].as<bool>();
    Info << "---[preciceAdapter]   checkpointing : " << checkpointingEnabled_ << nl;
}

// ************************************************************************* //
