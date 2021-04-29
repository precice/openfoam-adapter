#include "CHT.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::CHT::ConjugateHeatTransfer::ConjugateHeatTransfer(
    const Foam::fvMesh& mesh)
: mesh_(mesh)
{
}

bool preciceAdapter::CHT::ConjugateHeatTransfer::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the CHT module..."));

    // Read the CHT-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }


    // NOTE: If you want to add a new solver type, which you can manually
    // specify in the configuration, add it here. See also the methods
    // addWriters() and addReaders().
    // Check the solver type and determine it if needed
    if (
        solverType_.compare("compressible") == 0 || solverType_.compare("incompressible") == 0 || solverType_.compare("basic") == 0)
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
        DEBUG(adapterInfo("Determining the solver type for the CHT module... (override by setting solverType to one of {compressible, incompressible, basic})"));
        solverType_ = determineSolverType();
    }

    return true;
}

bool preciceAdapter::CHT::ConjugateHeatTransfer::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary CHTdict = adapterConfig.subOrEmptyDict("CHT");

    // Read the solver type (if not specified, it is determined automatically)
    solverType_ = CHTdict.lookupOrDefault<word>("solverType", "");
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    // Read the name of the temperature field (if different)
    nameT_ = CHTdict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    temperature field name : " + nameT_));

    // Read the name of the conductivity parameter for basic solvers (if different)
    nameKappa_ = CHTdict.lookupOrDefault<word>("nameKappa", "k");
    DEBUG(adapterInfo("    conductivity name for basic solvers : " + nameKappa_));

    // Read the name of the density parameter for incompressible solvers (if different)
    nameRho_ = CHTdict.lookupOrDefault<word>("nameRho", "rho");
    DEBUG(adapterInfo("    density name for incompressible solvers : " + nameRho_));

    // Read the name of the heat capacity parameter for incompressible solvers (if different)
    nameCp_ = CHTdict.lookupOrDefault<word>("nameCp", "Cp");
    DEBUG(adapterInfo("    heat capacity name for incompressible solvers : " + nameCp_));

    // Read the name of the Prandtl number parameter for incompressible solvers (if different)
    namePr_ = CHTdict.lookupOrDefault<word>("namePr", "Pr");
    DEBUG(adapterInfo("    Prandtl number name for incompressible solvers : " + namePr_));

    // Read the name of the turbulent thermal diffusivity field for incompressible solvers (if different)
    nameAlphat_ = CHTdict.lookupOrDefault<word>("nameAlphat", "alphat");
    DEBUG(adapterInfo("    Turbulent thermal diffusivity field name for incompressible solvers : " + nameAlphat_));

    return true;
}

std::string preciceAdapter::CHT::ConjugateHeatTransfer::determineSolverType()
{
    // NOTE: When coupling a different variable, you may want to
    // add more cases here. Or you may provide the solverType in the config.

    std::string solverType = "unknown";

    dimensionSet pressureDimensionsCompressible(1, -1, -2, 0, 0, 0, 0);
    dimensionSet pressureDimensionsIncompressible(0, 2, -2, 0, 0, 0, 0);

    if (mesh_.foundObject<volScalarField>("p"))
    {
        volScalarField p_ = mesh_.lookupObject<volScalarField>("p");

        if (p_.dimensions() == pressureDimensionsCompressible)
        {
            solverType = "compressible";
        }
        else if (p_.dimensions() == pressureDimensionsIncompressible)
        {
            solverType = "incompressible";
        }
    }

    if (solverType == "unknown")
    {
        adapterInfo("Failed to determine the solver type. "
                    "Please specify your solver type in the CHT section of the "
                    "preciceDict. Known solver types for CHT are: "
                    "basic, incompressible and "
                    "compressible",
                    "error");
    }

    DEBUG(adapterInfo("Automatically determined solver type : " + solverType));

    return solverType;
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addWriters(std::string dataName, Interface* interface)
{
    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new SinkTemperature(mesh_, nameT_));
        DEBUG(adapterInfo("Added writer: Sink Temperature."));
    }
    else if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Temperature(mesh_, nameT_));
        DEBUG(adapterInfo("Added writer: Temperature."));
    }
    else if (dataName.find("Heat-Flux") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataWriter(
                dataName,
                new HeatFlux_Compressible(mesh_, nameT_));
            DEBUG(adapterInfo("Added writer: Heat Flux for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataWriter(
                dataName,
                new HeatFlux_Incompressible(mesh_, nameT_, nameRho_, nameCp_, namePr_, nameAlphat_));
            DEBUG(adapterInfo("Added writer: Heat Flux for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataWriter(
                dataName,
                new HeatFlux_Basic(mesh_, nameT_, nameKappa_));
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
            interface->addCouplingDataWriter(
                dataName,
                new HeatTransferCoefficient_Compressible(mesh_, nameT_));
            DEBUG(adapterInfo("Added writer: Heat Transfer Coefficient for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataWriter(
                dataName,
                new HeatTransferCoefficient_Incompressible(mesh_, nameT_, nameRho_, nameCp_, namePr_, nameAlphat_));
            DEBUG(adapterInfo("Added writer: Heat Transfer Coefficient for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataWriter(
                dataName,
                new HeatTransferCoefficient_Basic(mesh_, nameT_, nameKappa_));
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
        adapterInfo("Unknown data type - cannot add " + dataName + ".", "error");
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::CHT::ConjugateHeatTransfer::addReaders(std::string dataName, Interface* interface)
{
    if (dataName.find("Sink-Temperature") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new SinkTemperature(mesh_, nameT_));
        DEBUG(adapterInfo("Added reader: Sink Temperature."));
    }
    else if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Temperature(mesh_, nameT_));
        DEBUG(adapterInfo("Added reader: Temperature."));
    }
    else if (dataName.find("Heat-Flux") == 0)
    {
        if (solverType_.compare("compressible") == 0)
        {
            interface->addCouplingDataReader(
                dataName,
                new HeatFlux_Compressible(mesh_, nameT_));
            DEBUG(adapterInfo("Added reader: Heat Flux for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataReader(
                dataName,
                new HeatFlux_Incompressible(mesh_, nameT_, nameRho_, nameCp_, namePr_, nameAlphat_));
            DEBUG(adapterInfo("Added reader: Heat Flux for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataReader(
                dataName,
                new HeatFlux_Basic(mesh_, nameT_, nameKappa_));
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
            interface->addCouplingDataReader(
                dataName,
                new HeatTransferCoefficient_Compressible(mesh_, nameT_));
            DEBUG(adapterInfo("Added reader: Heat Transfer Coefficient for compressible solvers."));
        }
        else if (solverType_.compare("incompressible") == 0)
        {
            interface->addCouplingDataReader(
                dataName,
                new HeatTransferCoefficient_Incompressible(mesh_, nameT_, nameRho_, nameCp_, namePr_, nameAlphat_));
            DEBUG(adapterInfo("Added reader: Heat Transfer Coefficient for incompressible solvers. "));
        }
        else if (solverType_.compare("basic") == 0)
        {
            interface->addCouplingDataReader(
                dataName,
                new HeatTransferCoefficient_Basic(mesh_, nameT_, nameKappa_));
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
        adapterInfo("Unknown data type - cannot add " + dataName + ".", "error");
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // reader here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
