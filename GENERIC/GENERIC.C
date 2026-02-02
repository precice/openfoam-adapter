#include "GENERIC.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::GENERIC::GenericInterface::GenericInterface(
    const Foam::fvMesh& mesh)
: mesh_(mesh) {}

bool preciceAdapter::GENERIC::GenericInterface::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the GENERIC module..."));

    // Read the GENERIC-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }

    return true;
}

bool preciceAdapter::GENERIC::GenericInterface::readConfig(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Reading GENERIC module configuration..."));

    // const dictionary& moduleDict = adapterConfig.subOrEmptyDict("GENERIC");

    // TODO: Reduce code duplication and get the interfaces configuration from preciceAdapter:Adapter

    const dictionary* interfaceDictPtr = adapterConfig.findDict("interfaces");
    struct InterfaceConfig interfaceConfig;

    for (const entry& interfaceDictEntry : *interfaceDictPtr)
    {
        if (interfaceDictEntry.isDict())
        {
            const dictionary& interfaceDict = interfaceDictEntry.dict();

            DEBUG(adapterInfo("    writeData    : "));
            const wordList& writeDataDict = interfaceDict.get<wordList>("writeData");

            for (const word& writeDatumEntry : writeDataDict)
            {
                word dataName = writeDatumEntry;

                // Assume dataName is same as solver_name, see code for new schema below
                struct fieldConfig fieldConfig;
                fieldConfig.name = dataName;
                fieldConfig.solver_name = dataName;
                fieldConfig.operation = "value";

                interfaceConfig.writeData.push_back(fieldConfig);
                DEBUG(adapterInfo("      - " + fieldConfig.name));
            }

            DEBUG(adapterInfo("    readData     : "));
            const wordList& readDataDict = interfaceDict.get<wordList>("readData");
            for (const word& readDatumEntry : readDataDict)
            {
                word dataName = readDatumEntry;

                struct fieldConfig fieldConfig;
                fieldConfig.name = dataName;
                fieldConfig.solver_name = dataName;
                fieldConfig.operation = "value";

                interfaceConfig.readData.push_back(fieldConfig);
                DEBUG(adapterInfo("      - " + fieldConfig.name));
            }
            interfacesConfig_.push_back(interfaceConfig);
        }
    }

    return true;
}

bool preciceAdapter::GENERIC::GenericInterface::addWriters(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    // Determine type of field
    bool isScalarField;

    // TODO: refactor to get rid of the inner loop

    // for all interfaces
    for (uint i = 0; i < interfacesConfig_.size(); i++)
    {
        // find the writer corresponding to dataName
        for (uint j = 0; j < interfacesConfig_.at(i).writeData.size(); j++)
        {
            const struct fieldConfig& fieldConfig = interfacesConfig_.at(i).writeData.at(j);
            if (fieldConfig.name == dataName)
            {

                if (mesh_.foundObject<volScalarField>(fieldConfig.solver_name))
                {
                    isScalarField = true;
                }
                else if (mesh_.foundObject<volVectorField>(fieldConfig.solver_name))
                {
                    isScalarField = false;
                }
                else
                {
                    found = false;
                    adapterInfo("Data " + dataName + " not found!");
                    return found;
                }

                if (isScalarField)
                {
                    interface->addCouplingDataWriter(
                        dataName,
                        new ScalarFieldCoupler(mesh_, fieldConfig));
                }
                else
                {
                    interface->addCouplingDataWriter(
                        dataName,
                        new VectorFieldCoupler(mesh_, fieldConfig));
                }

                DEBUG(adapterInfo("Added writer: " + dataName));
            }
        }
    }

    return found;
}

bool preciceAdapter::GENERIC::GenericInterface::addReaders(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    // Determine type of field
    bool isScalarField;

    for (uint i = 0; i < interfacesConfig_.size(); i++)
    {
        for (uint j = 0; j < interfacesConfig_.at(i).readData.size(); j++)
        {
            const struct fieldConfig& fieldConfig = interfacesConfig_.at(i).readData.at(j);
            if (fieldConfig.name == dataName)
            {

                if (mesh_.foundObject<volScalarField>(fieldConfig.solver_name))
                {
                    isScalarField = true;
                }
                else if (mesh_.foundObject<volVectorField>(fieldConfig.solver_name))
                {
                    isScalarField = false;
                }
                else
                {
                    found = false;
                    adapterInfo("Data " + dataName + " not found!");
                    return found;
                }

                if (isScalarField)
                {
                    interface->addCouplingDataReader(
                        dataName,
                        new ScalarFieldCoupler(mesh_, fieldConfig));
                }
                else
                {
                    interface->addCouplingDataReader(
                        dataName,
                        new VectorFieldCoupler(mesh_, fieldConfig));
                }

                DEBUG(adapterInfo("Added reader: " + dataName));
            }
        }
    }

    return found;
}
