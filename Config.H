#ifndef CONFIG_H
#define CONFIG_H

// Other header files
// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"

namespace preciceAdapter {

class Config
{

private:

    //- YAML Node object for the configuration file
    YAML::Node adapterConfig_;

    //- preCICE participant name
    std::string participantName_;

    //- preCICE configuration file name
    std::string preciceConfigFilename_;

    //- Switch for subcycling
    bool subcyclingAllowed_;

    //- Switch for checkpointing
    bool checkpointingEnabled_;

public:

    //- Constructor
    Config();

    //- Read the adapter's configuration file
    bool configFileRead();

    //- Check the adapter's configuration file
    bool configFileCheck(const std::string adapterConfigFileName);

    // Getters

        //- Returns the participantName_
        std::string participantName();

        //- Returns the preciceConfigFilename_
        std::string preciceConfigFilename();

        //- Returns the subcyclingAllowed_
        bool subcyclingAllowed();

        //- Returns the checkpointingEnabled_
        bool checkpointingEnabled();
};

}

#endif
