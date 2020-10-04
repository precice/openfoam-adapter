
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
    const Foam::fvMesh& mesh,
    const preciceAdapter::CHT::CHTParameterNames pnames
)
: mesh_(mesh), parameterName_(pnames)
{}

std::optional<std::shared_ptr<preciceAdapter::CHT::ConjugateHeatTransfer>>
preciceAdapter::CHT::ConjugateHeatTransfer::ConjugateHeatTransfer::CreateInstance
(
    const Foam::fvMesh& mesh,
    const IOdictionary& adapterConfig
)
{
    DEBUG(adapterInfo("Configuring the CHT module..."));

    // Read the CHT-specific options from the adapter's configuration file

    const dictionary CHTdict = adapterConfig.subOrEmptyDict("CHT");
    preciceAdapter::CHT::CHTParameterNames pnames;

    // Read the name of the temperature field (if different)
    pnames.T = CHTdict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    temperature field name : " + pnames.T));

    // // Read the name of the conductivity parameter for basic solvers (if different)
    // pnames.Kappa = CHTdict.lookupOrDefault<word>("nameKappa", "k");
    // DEBUG(adapterInfo("    conductivity name for basic solvers : " + pnames.Kappa));

    // // Read the name of the density parameter for incompressible solvers (if different)
    // pnames.Rho = CHTdict.lookupOrDefault<word>("nameRho", "rho");
    // DEBUG(adapterInfo("    density name for incompressible solvers : " + pnames.Rho));

    // // Read the name of the heat capacity parameter for incompressible solvers (if different)
    // pnames.Cp = CHTdict.lookupOrDefault<word>("nameCp", "Cp");
    // DEBUG(adapterInfo("    heat capacity name for incompressible solvers : " + pnames.Cp));

    // // Read the name of the Prandtl number parameter for incompressible solvers (if different)
    // pnames.Pr = CHTdict.lookupOrDefault<word>("namePr","Pr");
    // DEBUG(adapterInfo("    Prandtl number name for incompressible solvers : " + pnames.Pr));

    // // Read the name of the turbulent thermal diffusivity field for incompressible solvers (if different)
    // pnames.Alphat = CHTdict.lookupOrDefault<word>("nameAlphat", "alphat");
    // DEBUG(adapterInfo("    Turbulent thermal diffusivity field name for incompressible solvers : " + pnames.Alphat));

    return std::make_shared<ConjugateHeatTransfer>(mesh, pnames);
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addWriters
(
    std::string dataName,
    Interface * interface
)
{
    if
    (
        dataName.find("Sink-Temperature") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<SinkTemperature>(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Sink-Temperature."));
    }
    else if
    (
        dataName.find("Temperature") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<Temperature>(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Temperature."));
    }
    else if
    (
        dataName.find("Heat-Flux") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<HeatFlux>(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added writer: Heat-Flux."));
    }
    else if
    (
        dataName.find("Heat-Transfer-Coefficient") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<HeatTransferCoefficient>(mesh_, parameterName_.T)
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

void preciceAdapter::CHT::ConjugateHeatTransfer::addReaders
(
    std::string dataName,
    Interface * interface
)
{
    if
    (
        dataName.find("Sink-Temperature") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<SinkTemperature>(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Sink-Temperature."));
    }
    else if
    (
        dataName.find("Temperature") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<Temperature>(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Temperature."));
    }
    else if
    (
        dataName.find("Heat-Flux") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<HeatFlux>(mesh_, parameterName_.T)
        );
        DEBUG(adapterInfo("Added reader: Heat-Flux."));
    }
    else if
    (
        dataName.find("Heat-Transfer-Coefficient") == 0
    )
    {
        interface->addCouplingDataReader
        (
            dataName,
            std::make_shared<HeatTransferCoefficient>(mesh_, parameterName_.T)
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
