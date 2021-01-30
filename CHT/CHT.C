
#include "Interface.H"
#include "CHT.H"
#include "Utilities.H"

#include "CHT/Temperature.H"
#include "CHT/HeatFlux.H"
#include "CHT/SinkTemperature.H"
#include "CHT/HeatTransferCoefficient.H"

#include "fvCFD.H"

using namespace Foam;

preciceAdapter::CHT::ConjugateHeatTransfer::ConjugateHeatTransfer
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

bool preciceAdapter::CHT::ConjugateHeatTransfer::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the CHT module..."));

    // Read the CHT-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    return true;
}

bool preciceAdapter::CHT::ConjugateHeatTransfer::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary CHTdict = adapterConfig.subOrEmptyDict("CHT");

    // Read the name of the temperature field (if different)
    parameterName_.T = CHTdict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    temperature field name : " + parameterName_.T));

    return true;
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new SinkTemperature(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Sink Temperature."));
    }
    else if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Temperature(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Temperature."));
    }
    else if (dataName.find("Heat-Flux") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new HeatFlux(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Heat-Flux."));
    }
    else if (dataName.find("Heat-Transfer-Coefficient") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new HeatTransferCoefficient(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Heat-Transfer-Coefficient."));
    }
    else
    {
        adapterInfo("Unknown data type - cannot add " + dataName +".", "error");
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addReaders(std::string dataName, Interface * interface)
{
    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new SinkTemperature(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Sink Temperature."));
    }
    else if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Temperature(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Temperature."));
    }
    else if (dataName.find("Heat-Flux") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new HeatFlux(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Heat-Flux."));
    }
    else if (dataName.find("Heat-Transfer-Coefficient") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new HeatTransferCoefficient(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Heat-Transfer-Coefficient."));
    }
    else
    {
        adapterInfo("Unknown data type - cannot add " + dataName +".", "error");
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // reader here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
