#include "Config.H"

#include "IOstreams.H"

preciceAdapter::Config::Config()
{
    Foam::Info << "Entered the Config() constructor." << Foam::nl;
    return;
}


bool preciceAdapter::Config::configFileCheck(const std::string adapterConfigFileName)
{
    Foam::Info << "---[preciceAdapter] Check the adapter's YAML configuration file." << Foam::nl;

    bool configErrors = false;

    YAML::Node adapterConfig = YAML::LoadFile(adapterConfigFileName);

    // TODO Consider simplifying
    // Check if the "participant" node exists
    if ( !adapterConfig["participant"] )
    {
        Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: participant node is missing." << Foam::nl;
        configErrors = true;
    }

    // Check if the "precice-config-file" node exists
    if ( !adapterConfig["precice-config-file"] )
    {
        Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: 'precice-config-file' node is missing." << Foam::nl;
        configErrors = true;
        // TODO Check if the specified file exists
    }

    // Check if the "interfaces" node exists
    if ( !adapterConfig["interfaces"] )
    {
        Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: 'interfaces' node is missing." << Foam::nl;
        configErrors = true;
    } else {
        for ( uint i = 0; i < adapterConfig["interfaces"].size(); i++ )
        {
            if ( !adapterConfig["interfaces"][i]["mesh"] )
            {
                Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'mesh' node is missing for the interface " << i+1 << "." << Foam::nl;
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["patches"] )
            {
                Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'patches' node is missing for the interface " << i+1 << "." << Foam::nl;
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["write-data"] )
            {
                Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'write-data' node is missing for the interface " << i+1 << "." << Foam::nl;
                configErrors = true;
                // TODO Add check for allowed values.
            }
            if ( !adapterConfig["interfaces"][i]["read-data"] )
            {
                Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: the 'read-data' node is missing for the interface " << i+1 << "." << Foam::nl;
                configErrors = true;
                // TODO Add check for allowed values.
            }
        }
    }

    // Check if the "subcycling" node exists
    if ( !adapterConfig["subcycling"] )
    {
        Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: 'subcycling' node is missing." << Foam::nl;
        configErrors = true;
    }

    // Check if the "checkpointing" node exists
    if ( !adapterConfig["checkpointing"] )
    {
        Foam::Info << "---[preciceAdapter] Error: Adapter's configuration file: 'checkpointing' node is missing." << Foam::nl;
        configErrors = true;
    }

    if ( !configErrors )
    {
        Foam::Info << "---[preciceAdapter]   The adapter's YAML configuration file " << adapterConfigFileName << " is complete." << Foam::nl;
    }

    return !configErrors;
}


bool preciceAdapter::Config::configFileRead()
{
    Foam::Info << "---[preciceAdapter] Read the adapter's YAML configuration file (one per solver)." << Foam::nl;

    // Check the configuration file
    const std::string adapterConfigFileName = "precice-adapter-config.yml";
    if ( !configFileCheck(adapterConfigFileName) ) return false;

    // Load the YAML file
    adapterConfig_ = YAML::LoadFile(adapterConfigFileName);

    // Read the preCICE participant name
    participantName_ = adapterConfig_["participant"].as<std::string>();
    Foam::Info << "---[preciceAdapter]   participant : " << participantName_ << Foam::nl;

    // Read the preCICE configuration file name
    preciceConfigFilename_ = adapterConfig_["precice-config-file"].as<std::string>();
    Foam::Info << "---[preciceAdapter]   precice-config-file : " << preciceConfigFilename_ << Foam::nl;

    // TODO Read the coupling interfaces
    YAML::Node adapterConfigInterfaces = adapterConfig_["interfaces"];
    Foam::Info << "---[preciceAdapter]   interfaces : TODO " << Foam::nl;

    // Set the subcyclingAllowed_ switch
    subcyclingAllowed_ = adapterConfig_["subcycling"].as<bool>();
    Foam::Info << "---[preciceAdapter]   subcycling : " << subcyclingAllowed_ << Foam::nl;

    // Set the checkpointingEnabled_ switch
    checkpointingEnabled_ = adapterConfig_["checkpointing"].as<bool>();
    Foam::Info << "---[preciceAdapter]   checkpointing : " << checkpointingEnabled_ << Foam::nl;

    return true;
}

std::string preciceAdapter::Config::participantName()
{
    return participantName_;
}

std::string preciceAdapter::Config::preciceConfigFilename()
{
    return preciceConfigFilename_;
}

bool preciceAdapter::Config::subcyclingAllowed(){
    return subcyclingAllowed_;
}

bool preciceAdapter::Config::checkpointingEnabled(){
    return checkpointingEnabled_;
}
