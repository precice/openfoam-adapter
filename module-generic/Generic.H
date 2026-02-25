#ifndef GENERICMODULE_H
#define GENERICMODULE_H

#include "Interface.H"

#include "ReadWrite.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace Generic
{

class GenericInterface
{
protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Read the Generic related options from the adapter's configuration file
    bool readConfig(const IOdictionary& adapterConfig);

    //- Struct to store interfaces configuration (modified from Adapter.C)
    struct InterfaceConfig
    {
        std::vector<struct FieldConfig> writeData;
        std::vector<struct FieldConfig> readData;
    };

    //- Configuration interfaces
    std::vector<struct InterfaceConfig> interfacesConfig_;

    //- List of available fields in the OpenFOAM object registry
    std::string availableVolScalarFields;
    std::string availableVolVectorFields;

public:
    //- Constructor
    GenericInterface(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const IOdictionary& adapterConfig);

    //- Add coupling data writers
    bool addWriters(const preciceAdapter::FieldConfig& fieldConfig, Interface* interface);

    //- Add coupling data readers
    bool addReaders(const preciceAdapter::FieldConfig& fieldConfig, Interface* interface);
};

}
}

#endif
