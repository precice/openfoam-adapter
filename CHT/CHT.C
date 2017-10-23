#include "CHT.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::CHT::ConjugateHeatTransfer::ConjugateHeatTransfer
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

bool preciceAdapter::CHT::ConjugateHeatTransfer::configure(const YAML::Node adapterConfig)
{
    DEBUG(adapterInfo("Configuring the CHT module..."));

    // Read the CHT-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    // Check the solver type and determine it if needed
    if (
        solverType_.compare("compressible") == 0 ||
        solverType_.compare("incompressible") == 0 ||
        solverType_.compare("basic") == 0
    )
    {
        DEBUG(adapterInfo("Known solver type: " + solverType_));
    }
    else if (solverType_.compare("none") == 0)
    {
        DEBUG(adapterInfo("Determining the solver type..."));
        solverType_ = determineSolverType();
    }
    else
    {
        DEBUG(adapterInfo("Unknown solver type. Determining the solver type..."));
        solverType_ = determineSolverType();
    }

    return true;
}

bool preciceAdapter::CHT::ConjugateHeatTransfer::readConfig(const YAML::Node adapterConfig)
{
    // Read the solver type (if not specified, it is determined automatically)
    if (adapterConfig["solverType"])
    {
        solverType_ = adapterConfig["solverType"].as<std::string>();
    }
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    // Read the name of the temperature field (if different)
    if (adapterConfig["nameT"])
    {
        nameT_ = adapterConfig["nameT"].as<std::string>();
    }
    DEBUG(adapterInfo("    temperature field name : " + nameT_));

    // Read the name of the transportProperties dictionary (if different)
    if (adapterConfig["nameTransportProperties"])
    {
        nameTransportProperties_ = adapterConfig["nameTransportProperties"].as<std::string>();
    }
    DEBUG(adapterInfo("    transportProperties name : " + nameTransportProperties_));

    // Read the name of the conductivity parameter for basic solvers (if different)
    if (adapterConfig["nameKappa"])
    {
        nameKappa_ = adapterConfig["nameKappa"].as<std::string>();
    }
    DEBUG(adapterInfo("    conductivity name for basic solvers : " + nameKappa_));

    // Read the name of the density parameter for incompressible solvers (if different)
    if (adapterConfig["nameRho"])
    {
        nameRho_ = adapterConfig["nameRho"].as<std::string>();
    }
    DEBUG(adapterInfo("    density name for incompressible solvers : " + nameRho_));

    // Read the name of the heat capacity parameter for incompressible solvers (if different)
    if (adapterConfig["nameCp"])
    {
        nameCp_ = adapterConfig["nameCp"].as<std::string>();
    }
    DEBUG(adapterInfo("    heat capacity name for incompressible solvers : " + nameCp_));

    // Read the name of the Prandtl number parameter for incompressible solvers (if different)
    if (adapterConfig["namePr"])
    {
        namePr_ = adapterConfig["namePr"].as<std::string>();
    }
    DEBUG(adapterInfo("    Prandtl number name for incompressible solvers : " + namePr_));

    // Read the name of the turbulent thermal diffusivity field for incompressible solvers (if different)
    if (adapterConfig["nameAlphat"])
    {
        nameAlphat_ = adapterConfig["nameAlphat"].as<std::string>();
    }
    DEBUG(adapterInfo("    Turbulent thermal diffusivity field name for incompressible solvers : " + nameAlphat_));

    return true;
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
        DEBUG(adapterInfo("Found the transportProperties dictionary."));
    }
    else
    {
        DEBUG(adapterInfo("Did not find the transportProperties dictionary."));
    }

    if (mesh_.foundObject<IOdictionary>(turbulenceModel::propertiesName))
    {
        turbulencePropertiesExists = true;
        DEBUG(adapterInfo("Found the " + turbulenceModel::propertiesName
            + " dictionary."));
    }
    else
    {
        DEBUG(adapterInfo("Did not find the " + turbulenceModel::propertiesName
            + " dictionary."));
    }

    if (mesh_.foundObject<IOdictionary>("thermophysicalProperties"))
    {
        thermophysicalPropertiesExists = true;
        DEBUG(adapterInfo("Found the thermophysicalProperties dictionary."));
    }
    else
    {
        DEBUG(adapterInfo("Did not find the thermophysicalProperties dictionary."));
    }

    if (turbulencePropertiesExists)
    {
        if (thermophysicalPropertiesExists)
        {
            solverType = "compressible";
            DEBUG(adapterInfo("This is a compressible flow solver, "
                "as turbulence and thermophysical properties are provided."));
        }
        else if (transportPropertiesExists)
        {
            solverType = "incompressible";
            DEBUG(adapterInfo("This is an incompressible flow solver, "
            "as turbulence and transport properties are provided."));
        }
        else
        {
            adapterInfo("Could not determine the solver type, or this is not "
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
            DEBUG(adapterInfo("This is a basic solver, as transport properties "
            "are provided, while turbulence or transport properties are not "
            "provided."));
        }
        else
        {
            adapterInfo("Could not determine the solver type, or this is not a "
            "compatible solver: neither transport, nor turbulence properties "
            "are provided.",
            "error");
        }
    }

    return solverType;
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new SinkTemperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added writer: Sink Temperature."));
    }
    else if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added writer: Temperature."));
    }
    else if (dataName.find("Heat-Flux") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatFlux_Compressible(mesh_, nameT_)
            );
            DEBUG(adapterInfo("Added writer: Heat Flux for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatFlux_Incompressible(mesh_, nameT_, nameTransportProperties_, nameRho_, nameCp_, namePr_, nameAlphat_)
            );
            DEBUG(adapterInfo("Added writer: Heat Flux for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatFlux_Basic(mesh_, nameT_, nameTransportProperties_, nameKappa_)
            );
            DEBUG(adapterInfo("Added writer: Heat Flux for basic solvers. "));
        }
        else
        {
            adapterInfo("Unknown solver type - cannot add heat flux.",
                "error");
        }
    }
    else if (dataName.find("Heat-Transfer-Coefficient") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatTransferCoefficient_Compressible(mesh_, nameT_)
            );
            DEBUG(adapterInfo("Added writer: Heat Transfer Coefficient for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatTransferCoefficient_Incompressible(mesh_, nameT_, nameTransportProperties_, nameRho_, nameCp_, namePr_, nameAlphat_)
            );
            DEBUG(adapterInfo("Added writer: Heat Transfer Coefficient for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataWriter
            (
                dataName,
                new HeatTransferCoefficient_Basic(mesh_, nameT_, nameTransportProperties_, nameKappa_)
            );
            DEBUG(adapterInfo("Added writer: Heat Transfer Coefficient for basic solvers. "));
        }
        else
        {
            adapterInfo("Unknown solver type - cannot add heat transfer coefficient.",
                "error");
        }
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
            new SinkTemperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added reader: Sink Temperature."));
    }
    else if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added reader: Temperature."));
    }
    else if (dataName.find("Heat-Flux") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatFlux_Compressible(mesh_, nameT_)
            );
            DEBUG(adapterInfo("Added reader: Heat Flux for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatFlux_Incompressible(mesh_, nameT_, nameTransportProperties_, nameRho_, nameCp_, namePr_, nameAlphat_)
            );
            DEBUG(adapterInfo("Added reader: Heat Flux for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatFlux_Basic(mesh_, nameT_, nameTransportProperties_, nameKappa_)
            );
            DEBUG(adapterInfo("Added reader: Heat Flux for basic solvers. "));
        }
        else
        {
            adapterInfo("Unknown solver type - cannot add heat flux.",
                "error");
        }
    }
    else if (dataName.find("Heat-Transfer-Coefficient") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatTransferCoefficient_Compressible(mesh_, nameT_)
            );
            DEBUG(adapterInfo("Added reader: Heat Transfer Coefficient for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatTransferCoefficient_Incompressible(mesh_, nameT_, nameTransportProperties_, nameRho_, nameCp_, namePr_, nameAlphat_)
            );
            DEBUG(adapterInfo("Added reader: Heat Transfer Coefficient for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataReader
            (
                dataName,
                new HeatTransferCoefficient_Basic(mesh_, nameT_, nameTransportProperties_, nameKappa_)
            );
            DEBUG(adapterInfo("Added reader: Heat Transfer Coefficient for basic solvers. "));
        }
        else
        {
            adapterInfo("Unknown solver type - cannot add heat transfer coefficient.",
                "error");
        }
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
