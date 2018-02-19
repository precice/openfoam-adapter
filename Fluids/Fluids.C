#include "Fluids.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::Fluids::VelocityAndPressure::VelocityAndPressure
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

bool preciceAdapter::Fluids::VelocityAndPressure::configure(const YAML::Node adapterConfig)
{
    DEBUG(adapterInfo("Configuring the Fluids module..."));

    // Read the Fluids-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    // NOTE: If you want to add a new solver type, which you can manually
    // specify in the configuration, add it here. See also the methods
    // addWriters() and addReaders().
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

bool preciceAdapter::Fluids::VelocityAndPressure::readConfig(const YAML::Node adapterConfig)
{
    // Read the solver type (if not specified, it is determined automatically)
    if (adapterConfig["solverType"])
    {
        solverType_ = adapterConfig["solverType"].as<std::string>();
    }
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    // Read the name of the pressure field (if different)
    if (adapterConfig["nameP"])
    {
        nameP_ = adapterConfig["nameP"].as<std::string>();
    }
    DEBUG(adapterInfo("    pressure field name : " + nameP_));

    // Read the name of the transportProperties dictionary (if different)
    if (adapterConfig["nameTransportProperties"])
    {
        nameTransportProperties_ = adapterConfig["nameTransportProperties"].as<std::string>();
    }
    DEBUG(adapterInfo("    transportProperties name : " + nameTransportProperties_));

    // Read the name of the velocity field (if different)
    if (adapterConfig["nameU"])
    {
        nameU_ = adapterConfig["nameU"].as<std::string>();
    }
    DEBUG(adapterInfo("    velocity field name : " + nameU_));

    // Read the name of the density parameter for incompressible solvers (if different)
    if (adapterConfig["nameRho"])
    {
        nameRho_ = adapterConfig["nameRho"].as<std::string>();
    }
    DEBUG(adapterInfo("    density name for incompressible solvers : " + nameRho_));

    // // Read the name of the heat capacity parameter for incompressible solvers (if different)
    // if (adapterConfig["nameCp"])
    // {
    //     nameCp_ = adapterConfig["nameCp"].as<std::string>();
    // }
    // DEBUG(adapterInfo("    heat capacity name for incompressible solvers : " + nameCp_));

    // // Read the name of the Prandtl number parameter for incompressible solvers (if different)
    // if (adapterConfig["namePr"])
    // {
    //     namePr_ = adapterConfig["namePr"].as<std::string>();
    // }
    // DEBUG(adapterInfo("    Prandtl number name for incompressible solvers : " + namePr_));

    // // Read the name of the turbulent thermal diffusivity field for incompressible solvers (if different)
    // if (adapterConfig["nameAlphat"])
    // {
    //     nameAlphat_ = adapterConfig["nameAlphat"].as<std::string>();
    // }
    // DEBUG(adapterInfo("    Turbulent thermal diffusivity field name for incompressible solvers : " + nameAlphat_));

    return true;
}

std::string preciceAdapter::Fluids::VelocityAndPressure::determineSolverType()
{
    // NOTE: When coupling a different variable, you may want to
    // add more cases here. Or you may provide the solverType in the config.

    std::string solverType;

    // Determine the solver type: Compressible, Incompressible or Basic.
    // Look for the files transportProperties, turbulenceProperties,
    // and thermophysicalProperties
    bool transportPropertiesExists = false;
    bool turbulencePropertiesExists = false;
    bool thermophysicalPropertiesExists = false;

//    if (mesh_.foundObject<IOdictionary>(nameTransportProperties_))
//    {
//        transportPropertiesExists = true;
//        DEBUG(adapterInfo("Found the transportProperties dictionary."));
//    }
//    else
//    {
//        DEBUG(adapterInfo("Did not find the transportProperties dictionary."));
//    }
//
//    if (mesh_.foundObject<IOdictionary>(turbulenceModel::propertiesName))
//    {
//        turbulencePropertiesExists = true;
//        DEBUG(adapterInfo("Found the " + turbulenceModel::propertiesName
//            + " dictionary."));
//    }
//    else
//    {
//        DEBUG(adapterInfo("Did not find the " + turbulenceModel::propertiesName
//            + " dictionary."));
//    }
//
//    if (mesh_.foundObject<IOdictionary>("thermophysicalProperties"))
//    {
//        thermophysicalPropertiesExists = true;
//        DEBUG(adapterInfo("Found the thermophysicalProperties dictionary."));
//    }
//    else
//    {
//        DEBUG(adapterInfo("Did not find the thermophysicalProperties dictionary."));
//    }

    // I hack this so it works for the only solver I'm going to use for the moment.
    turbulencePropertiesExists = true;
    thermophysicalPropertiesExists = false;
    transportPropertiesExists = true;

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

void preciceAdapter::Fluids::VelocityAndPressure::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("p") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Pressure(&mesh_, nameP_)
        );
        DEBUG(adapterInfo("Added writer: Pressure."));
    }
    else if (dataName.find("U") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Velocity(&mesh_, nameU_)
        );
        DEBUG(adapterInfo("Added writer: Velocity."));
    }
    else
    {
        adapterInfo("Unknown data type - cannot add " + dataName +".", "error");
    }

    // TODO: Add the turbulence variables, maybe I'll have to do the if / else to check the type of the current solver
    // if I decide I want to allow the adapter to work for more than one solver.

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::Fluids::VelocityAndPressure::addReaders(std::string dataName, Interface * interface)
{
    if (dataName.find("Pressure") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Pressure(&mesh_, nameP_)
        );
        DEBUG(adapterInfo("Added reader: Pressure."));
    }
    else if (dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Velocity(&mesh_, nameU_)
        );
        DEBUG(adapterInfo("Added reader: Velocity."));
    }
    else
    {
        adapterInfo("Unknown data type - cannot add " + dataName +".", "error");
    }

    // TODO: Add the turbulence variables, maybe I'll have to do the if / else to check the type of the current solver
    // if I decide I want to allow the adapter to work for more than one solver.

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // reader here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
