#include "MODULE.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::MODULE::GeneralInterface::GeneralInterface(
    const Foam::fvMesh& mesh)
: mesh_(mesh) {}

bool preciceAdapter::MODULE::GeneralInterface::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the GENERAL module..."));

    // Read the GENERAL-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }

    return true;
}

bool preciceAdapter::MODULE::GeneralInterface::readConfig(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Reading GENERAL module configuration..."));

    // const dictionary& moduleDict = adapterConfig.subOrEmptyDict("MODULE");

    // TODO: Reduce code duplication and get the interfaces configuration from preciceAdapter:Adapter

    const dictionary* interfaceDictPtr = adapterConfig.findDict("interfaces");
    struct InterfaceConfig interfaceConfig;

    for (const entry& interfaceDictEntry : *interfaceDictPtr)
    {
        if (interfaceDictEntry.isDict())
        {
            const dictionary& interfaceDict = interfaceDictEntry.dict();

            DEBUG(adapterInfo("    writeData    : "));
            // Get writeData as a dictionary
            const dictionary& writeDataDict = interfaceDict.subDict("writeData");
            for (const entry& writeDatumEntry : writeDataDict)
            {

                const dictionary& writeDatumDict = writeDatumEntry.dict();
                word dataName = writeDatumEntry.keyword();

                struct fieldConfig fieldConfig;
                fieldConfig.name = dataName;
                fieldConfig.solver_name = writeDatumDict.lookupOrDefault<word>("solver_name", dataName); // default solver_name is the same
                fieldConfig.operation = writeDatumDict.lookupOrDefault<word>("operation", "value");      // default operation is "value"

                interfaceConfig.writeData.push_back(fieldConfig);

                DEBUG(adapterInfo("      - " + dataName));
                DEBUG(adapterInfo("        solver_name: " + fieldConfig.solver_name));
                DEBUG(adapterInfo("        operation  : " + fieldConfig.operation));
            }

            DEBUG(adapterInfo("    readData     : "));
            const dictionary& readDataDict = interfaceDict.subDict("readData");
            for (const entry& readDatumEntry : readDataDict)
            {
                const dictionary& readDatumDict = readDatumEntry.dict();
                word dataName = readDatumEntry.keyword();

                struct fieldConfig fieldConfig;
                fieldConfig.name = dataName;
                fieldConfig.solver_name = readDatumDict.lookupOrDefault<word>("solver_name", dataName); // default solver_name is the same
                fieldConfig.operation = readDatumDict.lookupOrDefault<word>("operation", "value");      // default operation is "value"

                interfaceConfig.readData.push_back(fieldConfig);

                DEBUG(adapterInfo("      - " + dataName));
                DEBUG(adapterInfo("        solver_name: " + fieldConfig.solver_name));
                DEBUG(adapterInfo("        operation  : " + fieldConfig.operation));
            }
            interfacesConfig_.push_back(interfaceConfig);
        }
    }

    return true;
}

bool preciceAdapter::MODULE::GeneralInterface::addWriters(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    // TODO: refactor to get rid of the loop
    // TODO: determine type of data (scalar, vector, etc.) from lookupObject

    // for all interfaces
    for (uint i = 0; i < interfacesConfig_.size(); i++)
    {
        // find the writer corresponding to dataName
        for (uint j = 0; j < interfacesConfig_.at(i).writeData.size(); j++)
        {
            const struct fieldConfig& fieldConfig = interfacesConfig_.at(i).writeData.at(j);
            if (fieldConfig.name == dataName)
            {
                interface->addCouplingDataWriter(
                    dataName,
                    new ScalarFieldCoupler(mesh_, fieldConfig));

                DEBUG(adapterInfo("Added writer: " + dataName));
            }
        }
    }

    return found;
}

bool preciceAdapter::MODULE::GeneralInterface::addReaders(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    for (uint i = 0; i < interfacesConfig_.size(); i++)
    {
        for (uint j = 0; j < interfacesConfig_.at(i).readData.size(); j++)
        {
            const struct fieldConfig& fieldConfig = interfacesConfig_.at(i).readData.at(j);
            if (fieldConfig.name == dataName)
            {
                interface->addCouplingDataReader(
                    dataName,
                    new ScalarFieldCoupler(mesh_, fieldConfig));

                DEBUG(adapterInfo("Added reader: " + dataName));
            }
        }
    }

    return found;
}
