#include "CHT.H"
#include "Adapter.H"

using namespace Foam;

// Output debug messages. Keep it disabled for production runs.
#define ADAPTER_DEBUG_MODE

#ifdef ADAPTER_DEBUG_MODE
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

preciceAdapter::CHT::ConjugateHeatTransfer::ConjugateHeatTransfer
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

void preciceAdapter::CHT::ConjugateHeatTransfer::configure
(
    const std::string nameTransportProperties,
    const std::string nameT
)
{
    DEBUG(Adapter::adapterInfo("Configuring the CHT module..."));

    nameTransportProperties_ = nameTransportProperties;
    nameT_ = nameT;

    solverType_ = determineSolverType();
}

std::string preciceAdapter::CHT::ConjugateHeatTransfer::determineSolverType()
{
    // NOTE: When coupling a different variable, you may want to
    // add more cases here.

    std::string solverType;

    // Determine the solver type: Compressible, Incompressible or Basic.
    // Look for the files transportProperties, turbulenceProperties,
    // and thermophysicalProperties
    bool transportPropertiesExists = false;
    bool turbulencePropertiesExists = false;
    bool thermophysicalPropertiesExists = false;

    if (mesh_.foundObject<IOdictionary>(nameTransportProperties_))
    {
        transportPropertiesExists = true;
        DEBUG(Adapter::adapterInfo("Found the transportProperties dictionary."));
    }
    else
    {
        DEBUG(Adapter::adapterInfo("Did not find the transportProperties dictionary."));
    }

    if (mesh_.foundObject<IOdictionary>(turbulenceModel::propertiesName))
    {
        turbulencePropertiesExists = true;
        DEBUG(Adapter::adapterInfo("Found the " + turbulenceModel::propertiesName
            + " dictionary."));
    }
    else
    {
        DEBUG(Adapter::adapterInfo("Did not find the " + turbulenceModel::propertiesName
            + " dictionary."));
    }

    if (mesh_.foundObject<IOdictionary>("thermophysicalProperties"))
    {
        thermophysicalPropertiesExists = true;
        DEBUG(Adapter::adapterInfo("Found the thermophysicalProperties dictionary."));
    }
    else
    {
        DEBUG(Adapter::adapterInfo("Did not find the thermophysicalProperties dictionary."));
    }

    if (turbulencePropertiesExists)
    {
        if (thermophysicalPropertiesExists)
        {
            solverType = "compressible";
            DEBUG(Adapter::adapterInfo("This is a compressible flow solver, "
                "as turbulence and thermophysical properties are provided."));
        }
        else if (transportPropertiesExists)
        {
            solverType = "incompressible";
            DEBUG(Adapter::adapterInfo("This is an incompressible flow solver, "
            "as turbulence and transport properties are provided."));
        }
        else
        {
            Adapter::adapterInfo("Could not determine the solver type, or this is not "
            "a compatible solver: although turbulence properties are provided, "
            "neither transport or thermophysical properties are provided.",
            "error");
        }
    }
    else
    {
        if (transportPropertiesExists)
        {
            solverType = "basic";
            DEBUG(Adapter::adapterInfo("This is a basic solver, as transport properties "
            "are provided, while turbulence or transport properties are not "
            "provided."));
        }
        else
        {
            Adapter::adapterInfo("Could not determine the solver type, or this is not a "
            "compatible solver: neither transport, nor turbulence properties "
            "are provided.",
            "error");
        }
    }

    return solverType;
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.compare("Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(Adapter::adapterInfo("  Added Temperature."));
    }

    if (dataName.compare("Heat-Flux") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatFlux_Compressible(mesh_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Flux for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatFlux_Incompressible(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Flux for incompressible solvers. "
                "Requires additional parameters to be read from the "
                "solver (see README)."));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatFlux_Basic(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Flux for basic solvers. "
                "Requires additional parameters to be read from the "
                "solver (see README)."));
        }
        else
        {
            Adapter::adapterInfo("Unknown solver type - cannot add heat flux.",
                "error");
        }
    }

    if (dataName.find("Heat-Transfer-Coefficient") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatTransferCoefficient_Compressible(mesh_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Transfer Coefficient for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatTransferCoefficient_Incompressible(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Transfer Coefficient for incompressible solvers. "
                "Requires additional parameters to be read from the "
                "solver (see README)."));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatTransferCoefficient_Basic(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Transfer Coefficient for basic solvers. "
                "Requires additional parameters to be read from the "
                "solver (see README)."));
        }
        else
        {
            Adapter::adapterInfo("Unknown solver type - cannot add heat transfer coefficient.",
                "error");
        }
    }

    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new SinkTemperature(mesh_, nameT_)
        );
        DEBUG(Adapter::adapterInfo("  Added Sink Temperature."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addReaders(std::string dataName, Interface * interface)
{
    if (dataName.compare("Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(Adapter::adapterInfo("  Added Temperature."));
    }

    if (dataName.compare("Heat-Flux") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatFlux_Compressible(mesh_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Flux for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatFlux_Incompressible(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Flux for incompressible solvers. "
                "Requires additional parameters to be read from the solver "
                "(see README)."));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatFlux_Basic(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Flux for basic solvers. "
                "Requires additional parameters to be read from the "
                "solver (see README)."));
        }
        else
        {
            Adapter::adapterInfo("Unknown solver type - cannot add heat flux.",
                "error");
        }
    }

    if (dataName.find("Heat-Transfer-Coefficient") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatTransferCoefficient_Compressible(mesh_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Transfer Coefficient for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatTransferCoefficient_Incompressible(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Transfer Coefficient for incompressible solvers. "
                "Requires additional parameters to be read from the solver "
                "(see README)."));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatTransferCoefficient_Basic(mesh_, nameTransportProperties_, nameT_)
            );
            DEBUG(Adapter::adapterInfo("  Added Heat Transfer Coefficient for basic solvers. "
                "Requires additional parameters to be read from the "
                "solver (see README)."));
        }
        else
        {
            Adapter::adapterInfo("Unknown solver type - cannot add heat transfer coefficient.",
                "error");
        }
    }

    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new SinkTemperature(mesh_, nameT_)
        );
        DEBUG(Adapter::adapterInfo("  Added Sink Temperature."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // reader here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
