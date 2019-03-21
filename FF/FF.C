#include "FF.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::FF::FluidFluid::FluidFluid
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

bool preciceAdapter::FF::FluidFluid::configure(const YAML::Node adapterConfig)
{
    DEBUG(adapterInfo("Configuring the CHT module..."));

    // Read the FF-specific options from the adapter's configuration file
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

bool preciceAdapter::FF::FluidFluid::readConfig(const YAML::Node adapterConfig)
{
    // Read the name of the velocity field (if different)
    if (adapterConfig["nameU"])
    {
        nameU_ = adapterConfig["nameU"].as<std::string>();
    }
    DEBUG(adapterInfo("    velocity field name : " + nameU_));

    // Read the name of the pressure field (if different)
    if (adapterConfig["nameP"])
    {
        nameP_ = adapterConfig["nameP"].as<std::string>();
    }
    DEBUG(adapterInfo("    pressure field name : " + nameP_));

    return true;
}

std::string preciceAdapter::FF::FluidFluid::determineSolverType()
{
    // NOTE: When coupling a different variable, you may want to
    // add more cases here. Or you may provide the solverType in the config.

    std::string solverType;
    
    // TODO: Implement properly
    solverType = "incompressible";
    
    return solverType;
}

void preciceAdapter::FF::FluidFluid::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Velocity(mesh_, nameU_)
        );
        DEBUG(adapterInfo("Added writer: Velocity."));
    }
    else if (dataName.find("PressureGradient") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new PressureGradient(mesh_, nameP_)
        );
        DEBUG(adapterInfo("Added writer: Pressure."));
    }
    else if (dataName.find("Pressure") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Pressure(mesh_, nameP_)
        );
        DEBUG(adapterInfo("Added writer: Pressure."));
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

void preciceAdapter::FF::FluidFluid::addReaders(std::string dataName, Interface * interface)
{
    if (dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Velocity(mesh_, nameU_)
        );
        DEBUG(adapterInfo("Added reader: Velocity."));
    }
    else if (dataName.find("PressureGradient") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new PressureGradient(mesh_, nameP_)
        );
        DEBUG(adapterInfo("Added reader: Pressure."));
    }
    else if (dataName.find("Pressure") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Pressure(mesh_, nameP_)
        );
        DEBUG(adapterInfo("Added reader: Pressure."));
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


