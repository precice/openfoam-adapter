#include "Config.H"

#include "IOstreams.H"

using namespace Foam;

preciceAdapter::Config::Config()
{
    return;
}


bool preciceAdapter::Config::configFileCheck(const std::string adapterConfigFileName)
{
    Info << "---[CONFIG] Check the adapter's YAML configuration file." << nl;

    bool configErrors = false;

    YAML::Node adapterConfig = YAML::LoadFile(adapterConfigFileName);

    // TODO Consider simplifying
    // Check if the "participant" node exists
    if ( !adapterConfig["participant"] )
    {
        Info << "---[CONFIG] Error: Adapter's configuration file: participant node is missing." << nl;
        configErrors = true;
    }

    // Check if the "precice-config-file" node exists
    if ( !adapterConfig["precice-config-file"] )
    {
        Info << "---[CONFIG] Error: Adapter's configuration file: 'precice-config-file' node is missing." << nl;
        configErrors = true;
        // TODO Check if the specified file exists
    }

    // Check if the "interfaces" node exists
    if ( !adapterConfig["interfaces"] )
    {
        Info << "---[CONFIG] Error: Adapter's configuration file: 'interfaces' node is missing." << nl;
        configErrors = true;
    } else {
        for ( uint i = 0; i < adapterConfig["interfaces"].size(); i++ )
        {
            if ( !adapterConfig["interfaces"][i]["mesh"] )
            {
                Info << "---[CONFIG] Error: Adapter's configuration file: the 'mesh' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["patches"] )
            {
                Info << "---[CONFIG] Error: Adapter's configuration file: the 'patches' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
            }
            if ( !adapterConfig["interfaces"][i]["write-data"] )
            {
                Info << "---[CONFIG] Error: Adapter's configuration file: the 'write-data' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
                // TODO Add check for allowed values.
            }
            if ( !adapterConfig["interfaces"][i]["read-data"] )
            {
                Info << "---[CONFIG] Error: Adapter's configuration file: the 'read-data' node is missing for the interface " << i+1 << "." << nl;
                configErrors = true;
                // TODO Add check for allowed values.
            }
        }
    }

    // Check if the "subcycling" node exists
    if ( !adapterConfig["subcycling"] )
    {
        Info << "---[CONFIG] Error: Adapter's configuration file: 'subcycling' node is missing." << nl;
        configErrors = true;
    }

    // Check if the "checkpointing" node exists
    if ( !adapterConfig["checkpointing"] )
    {
        Info << "---[CONFIG] Error: Adapter's configuration file: 'checkpointing' node is missing." << nl;
        configErrors = true;
    }

    if ( !configErrors )
    {
        Info << "---[CONFIG]   The adapter's YAML configuration file " << adapterConfigFileName << " is complete." << nl;
    }

    return !configErrors;
}


bool preciceAdapter::Config::configFileRead(const std::string casePath)
{

    // Check the configuration file
    const std::string adapterConfigFileName = casePath + "/precice-adapter-config.yml";
    Info << "---[CONFIG] Read the adapter's YAML configuration file " << adapterConfigFileName << "." << nl;

    if ( !configFileCheck(adapterConfigFileName) ) return false;

    // Load the YAML file
    adapterConfig_ = YAML::LoadFile(adapterConfigFileName);

    // Read the preCICE participant name
    participantName_ = adapterConfig_["participant"].as<std::string>();
    Info << "---[CONFIG]   participant : " << participantName_ << nl;

    // Read the preCICE configuration file name
    preciceConfigFilename_ = adapterConfig_["precice-config-file"].as<std::string>();
    Info << "---[CONFIG]   precice-config-file : " << preciceConfigFilename_ << nl;

    // TODO Read the coupling interfaces
    YAML::Node adapterConfigInterfaces = adapterConfig_["interfaces"];
    Info << "---[CONFIG]   interfaces : " << nl;
    for (uint i = 0; i < adapterConfigInterfaces.size(); i++)
    {
        struct Interface interface;
        interface.meshName = adapterConfigInterfaces[i]["mesh"].as<std::string>();
        Info << "---[CONFIG]     - mesh      : " << interface.meshName << nl;

        Info << "---[CONFIG]       patches   : ";
        for ( uint j = 0; j < adapterConfigInterfaces[i]["patches"].size(); j++)
        {
            interface.patchNames.push_back( adapterConfigInterfaces[i]["patches"][j].as<std::string>() );
            Info << adapterConfigInterfaces[i]["patches"][j].as<std::string>() << " ";
        }
         Info << nl;

        // TODO: Consider simplification
        if ( adapterConfigInterfaces[i]["write-data"] )
        {
            Info << "---[CONFIG]       writeData : ";
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
            Info << "---[CONFIG]       readData  : ";
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
    Info << "---[CONFIG]   subcycling : " << subcyclingAllowed_ << nl;

    // Set the checkpointingEnabled_ switch
    checkpointingEnabled_ = adapterConfig_["checkpointing"].as<bool>();
    Info << "---[CONFIG]   checkpointing : " << checkpointingEnabled_ << nl;

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
