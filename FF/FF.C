#include "FF.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::FF::FluidFluid::FluidFluid(
    const Foam::fvMesh& mesh)
: mesh_(mesh)
{
}

bool preciceAdapter::FF::FluidFluid::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the FF module..."));

    // Read the FF-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }

    // NOTE: If you want to add a new solver type, which you can manually
    // specify in the configuration, add it here. See also the methods
    // addWriters() and addReaders().
    // Check the solver type and determine it if needed
    if (
        solverType_.compare("compressible") == 0 || solverType_.compare("incompressible") == 0)
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
        DEBUG(adapterInfo("Determining the solver type for the FF module... (override by setting solverType to one of {compressible, incompressible})"));
        solverType_ = determineSolverType();
    }

    return true;
}

bool preciceAdapter::FF::FluidFluid::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary& FFdict = adapterConfig.subOrEmptyDict("FF");

    // Read the solver type (if not specified, it is determined automatically)
    solverType_ = FFdict.lookupOrDefault<word>("solverType", "");
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    // Read the name of the velocity field (if different)
    nameU_ = FFdict.lookupOrDefault<word>("nameU", "U");
    DEBUG(adapterInfo("    velocity field name : " + nameU_));

    // Read the name of the pressure field (if different)
    nameP_ = FFdict.lookupOrDefault<word>("nameP", "p");
    DEBUG(adapterInfo("    pressure field name : " + nameP_));

    // Read the name of the temperature field (if different)
    nameT_ = FFdict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    temperature field name : " + nameT_));

    // Read the name of the phase variable field (if different)
    nameAlpha_ = FFdict.lookupOrDefault<word>("nameAlpha", "alpha");
    DEBUG(adapterInfo("    phase variable (alpha) field name : " + nameAlpha_));

    // Read the name of the face flux field (if different)
    namePhi_ = FFdict.lookupOrDefault<word>("namePhi", "phi");
    DEBUG(adapterInfo("    face flux field name : " + namePhi_));

    // Check whether to enable flux correction for velocity
    fluxCorrection_ = FFdict.lookupOrDefault<bool>("fluxCorrection", false);
    DEBUG(adapterInfo("    flux correction of velocity is set to : " + std::to_string(fluxCorrection_)));

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
        const volScalarField& p_ = mesh_.lookupObject<volScalarField>("p");

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

bool preciceAdapter::FF::FluidFluid::addWriters(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    if (dataName.find("VelocityGradient") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new VelocityGradient(mesh_, nameU_));
        DEBUG(adapterInfo("Added writer: Velocity Gradient."));
    }
    else if (dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Velocity(mesh_, nameU_, namePhi_, fluxCorrection_));
        DEBUG(adapterInfo("Added writer: Velocity."));
    }
    else if (dataName.find("PressureGradient") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new PressureGradient(mesh_, nameP_));
        DEBUG(adapterInfo("Added writer: Pressure Gradient."));
    }
    else if (dataName.find("Pressure") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Pressure(mesh_, nameP_));
        DEBUG(adapterInfo("Added writer: Pressure."));
    }
    else if (dataName.find("FlowTemperatureGradient") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new TemperatureGradient(mesh_, nameT_));
        DEBUG(adapterInfo("Added writer: Flow Temperature Gradient."));
    }
    else if (dataName.find("FlowTemperature") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Temperature(mesh_, nameT_));
        DEBUG(adapterInfo("Added writer: Flow Temperature."));
    }
    else if (dataName.find("AlphaGradient") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new AlphaGradient(mesh_, nameAlpha_));
        DEBUG(adapterInfo("Added writer: Alpha Gradient."));
    }
    else if (dataName.find("Alpha") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Alpha(mesh_, nameAlpha_));
        DEBUG(adapterInfo("Added writer: Alpha."));
    }
    else if (dataName.find("Phi") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Phi(mesh_, namePhi_));
        DEBUG(adapterInfo("Added writer: Phi."));
    }
    else
    {
        found = false;
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.

    return found;
}

bool preciceAdapter::FF::FluidFluid::addReaders(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    if (dataName.find("VelocityGradient") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new VelocityGradient(mesh_, nameU_));
        DEBUG(adapterInfo("Added reader: VelocityGradient."));
    }
    else if (dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Velocity(mesh_, nameU_, namePhi_));
        DEBUG(adapterInfo("Added reader: Velocity."));
    }
    else if (dataName.find("PressureGradient") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new PressureGradient(mesh_, nameP_));
        DEBUG(adapterInfo("Added reader: Pressure Gradient."));
    }
    else if (dataName.find("Pressure") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Pressure(mesh_, nameP_));
        DEBUG(adapterInfo("Added reader: Pressure."));
    }
    else if (dataName.find("FlowTemperatureGradient") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new TemperatureGradient(mesh_, nameT_));
        DEBUG(adapterInfo("Added reader: Flow Temperature Gradient."));
    }
    else if (dataName.find("FlowTemperature") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Temperature(mesh_, nameT_));
        DEBUG(adapterInfo("Added reader: Flow Temperature."));
    }
    else if (dataName.find("AlphaGradient") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new AlphaGradient(mesh_, nameAlpha_));
        DEBUG(adapterInfo("Added reader: Alpha Gradient."));
    }
    else if (dataName.find("Alpha") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Alpha(mesh_, nameAlpha_));
        DEBUG(adapterInfo("Added reader: Alpha."));
    }
    else if (dataName.find("Phi") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Phi(mesh_, namePhi_));
        DEBUG(adapterInfo("Added reader: Phi."));
    }
    else
    {
        found = false;
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // reader here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.

    return found;
}
