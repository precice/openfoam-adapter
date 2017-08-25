#include "Config.H"

#include "IOstreams.H"

using namespace Foam;

preciceAdapter::Config::Config()
{
    Info << "Entered the Config() constructor." << nl;
    return;
}


bool preciceAdapter::Config::configFileCheck(const std::string adapterConfigFileName)
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

    if ( !configErrors )
    {
        Info << "---[preciceAdapter]   The adapter's YAML configuration file " << adapterConfigFileName << " is complete." << nl;
    }

    return !configErrors;
}


bool preciceAdapter::Config::configFileRead()
{
    Info << "---[preciceAdapter] Read the adapter's YAML configuration file (one per solver)." << nl;

    // Check the configuration file
    const std::string adapterConfigFileName = "precice-adapter-config.yml";
    if ( !configFileCheck(adapterConfigFileName) ) return false;

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
    Info << "---[preciceAdapter]   interfaces : " << nl;
    for (uint i = 0; i < adapterConfigInterfaces.size(); i++)
    {
        struct Interface interface;
        interface.meshName = adapterConfigInterfaces[i]["mesh"].as<std::string>();
        Info << "---[preciceAdapter]     - mesh      : " << interface.meshName << nl;

        Info << "---[preciceAdapter]       patches   : ";
        for ( uint j = 0; j < adapterConfigInterfaces[i]["patches"].size(); j++)
        {
            interface.patchNames.push_back( adapterConfigInterfaces[i]["patches"][j].as<std::string>() );
            Info << adapterConfigInterfaces[i]["patches"][j].as<std::string>() << " ";
        }
         Info << nl;

        // TODO: Consider simplification
        if ( adapterConfigInterfaces[i]["write-data"] )
        {
            Info << "---[preciceAdapter]       writeData : ";
            if ( adapterConfigInterfaces[i]["write-data"].size() > 0 )
            {
                for ( uint j = 0; j < adapterConfigInterfaces[i]["read-data"].size(); j++)
                {
                    interface.writeData.push_back( adapterConfigInterfaces[i]["write-data"][j].as<std::string>() );
                    Info << adapterConfigInterfaces[i]["write-data"][j].as<std::string>() << " ";
                }
            }
            else
            {
                interface.writeData.push_back( adapterConfigInterfaces[i]["write-data"].as<std::string>() );
                Info << adapterConfigInterfaces[i]["write-data"].as<std::string>() << " ";
            }
            Info << nl;
        }

        // TODO: Consider simplification
        if ( adapterConfigInterfaces[i]["read-data"] )
        {
            Info << "---[preciceAdapter]       readData  : ";
            if ( adapterConfigInterfaces[i]["read-data"].size() > 0 )
            {
                for ( uint j = 0; j < adapterConfigInterfaces[i]["read-data"].size(); j++)
                {
                    interface.readData.push_back( adapterConfigInterfaces[i]["read-data"][j].as<std::string>() );
                    Info << adapterConfigInterfaces[i]["read-data"][j].as<std::string>() << " ";
                }
            }
            else
            {
                interface.readData.push_back( adapterConfigInterfaces[i]["read-data"].as<std::string>() );
                Info << adapterConfigInterfaces[i]["read-data"].as<std::string>() << " ";
            }
            Info << nl;
        }

        interfaces_.push_back( interface );
    }

    // Set the subcyclingAllowed_ switch
    subcyclingAllowed_ = adapterConfig_["subcycling"].as<bool>();
    Info << "---[preciceAdapter]   subcycling : " << subcyclingAllowed_ << nl;

    // Set the checkpointingEnabled_ switch
    checkpointingEnabled_ = adapterConfig_["checkpointing"].as<bool>();
    Info << "---[preciceAdapter]   checkpointing : " << checkpointingEnabled_ << nl;

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

bool preciceAdapter::Config::subcyclingAllowed()
{
    return subcyclingAllowed_;
}

bool preciceAdapter::Config::checkpointingEnabled()
{
    return checkpointingEnabled_;
}

std::vector<struct preciceAdapter::Config::Interface> preciceAdapter::Config::interfaces()
{
    return interfaces_;
}
