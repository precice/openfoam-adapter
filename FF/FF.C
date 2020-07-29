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

bool preciceAdapter::FF::FluidFluid::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the FF module..."));

    // Read the FF-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    // NOTE: If you want to add a new solver type, which you can manually
    // specify in the configuration, add it here. See also the methods
    // addWriters() and addReaders().
    // Check the solver type and determine it if needed
    if (
        solverType_.compare("compressible") == 0 ||
        solverType_.compare("incompressible") == 0
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

bool preciceAdapter::FF::FluidFluid::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary FFdict = adapterConfig.subOrEmptyDict("FF");
    
    // Read the solver type (if not specified, it is determined automatically)
    solverType_ = FFdict.lookupOrDefault<word>("solverType", "");
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));
    
    // Read the name of the velocity field (if different)
    nameU_ = FFdict.lookupOrDefault<word>("nameU", "U");
    DEBUG(adapterInfo("    velocity field name : " + nameU_));

    // Read the name of the pressure field (if different)
    nameP_ = FFdict.lookupOrDefault<word>("nameP", "p");
    DEBUG(adapterInfo("    pressure field name : " + nameP_));

    return true;
}

std::string preciceAdapter::FF::FluidFluid::determineSolverType()
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
        solverType = "compressible";
      else if (p_.dimensions() == pressureDimensionsIncompressible)
        solverType = "incompressible";
      // TODO: Add special case for multiphase solvers.
      // Currently, interFoam is misclassified as "compressible".
    }

    if (solverType == "unknown")
      adapterInfo("Failed to determine the solver type. "
                  "Please specify your solver type in the FF section of the "
                  "preciceDict. Known solver types for FF are: "
                  "incompressible and "
                  "compressible",
                  "error");

    DEBUG(adapterInfo("Automatically determined solver type : " + solverType));

    return solverType;
}

void preciceAdapter::FF::FluidFluid::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("VelocityGradient") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new VelocityGradient(mesh_, nameU_)
        );
        DEBUG(adapterInfo("Added writer: Velocity Gradient."));
    }
    else if (dataName.find("Velocity") == 0)
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
        DEBUG(adapterInfo("Added writer: Pressure Gradient."));
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
    if (dataName.find("VelocityGradient") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new VelocityGradient(mesh_, nameU_)
        );
        DEBUG(adapterInfo("Added reader: VelocityGradient."));
    }
    else if (dataName.find("Velocity") == 0)
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
        DEBUG(adapterInfo("Added reader: Pressure Gradient."));
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

