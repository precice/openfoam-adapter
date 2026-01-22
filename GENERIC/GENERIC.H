#ifndef GENERICMODULE_H
#define GENERICMODULE_H

#include "Interface.H"

#include "GENERIC/ReadWrite.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace GENERIC
{

class GenericInterface
{
protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Read the GENERIC related options from the adapter's configuration file
    bool readConfig(const IOdictionary& adapterConfig);

    //- Struct to store interfaces configuration (modified from Adapter.C)
    struct InterfaceConfig
    {
        std::vector<struct fieldConfig> writeData;
        std::vector<struct fieldConfig> readData;
    };

    //- Configuration interfaces
    std::vector<struct InterfaceConfig> interfacesConfig_;

public:
    //- Constructor
    GenericInterface(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const IOdictionary& adapterConfig);

    //- Add coupling data writers
    bool addWriters(std::string dataName, Interface* interface);

    //- Add coupling data readers
    bool addReaders(std::string dataName, Interface* interface);
};

}
}

#endif
