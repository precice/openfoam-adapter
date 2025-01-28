#include "MODULE.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::MODULE::ConjugateHeatTransfer::ConjugateHeatTransfer(
    const Foam::fvMesh& mesh)
: mesh_(mesh) {}

bool preciceAdapter::MODULE::ConjugateHeatTransfer::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the GENERAL module..."));

    // Read the GENERAL-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }

    return true;
}

bool preciceAdapter::MODULE::ConjugateHeatTransfer::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary& ModuleDict = adapterConfig.subOrEmptyDict("MODULE");

    // Read the name of the temperature field (if different)
    nameT_ = ModuleDict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    temperature field name : " + nameT_));

    return true;
}

bool preciceAdapter::MODULE::ConjugateHeatTransfer::addWriters(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    // TODO: Init both temperature and heat flux at once? Also if dict contains Heat-Flux, not just Temperature. I.e., one or both.
    if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Temperature(mesh_, nameT_));

        interface->addCouplingDataWriter(
            dataName,
            new HeatFlux(mesh_, nameT_));

        DEBUG(adapterInfo("Added writer: Temperature."));
    }
    else
    {
        found = false;
    }

    return found;
}

bool preciceAdapter::MODULE::ConjugateHeatTransfer::addReaders(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Temperature(mesh_, nameT_));

        interface->addCouplingDataReader(
            dataName,
            new HeatFlux(mesh_, nameT_));

        DEBUG(adapterInfo("Added reader: Temperature."));
    }
    else
    {
        found = false;
    }

    return found;
}
