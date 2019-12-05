#include "AITS.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::AITS::AITSolver::AITSolver
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

bool preciceAdapter::AITS::AITSolver::configure(const YAML::Node adapterConfig)
{
    DEBUG(adapterInfo("Configuring the AITS module..."));

    // Read the AITS-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    // NOTE: If you want to add a new solver type, which you can manually
    // specify in the configuration, add it here. See also the methods
    // addWriters() and addReaders().
    // Check the solver type and determine it if needed
    cout << "SolverType: |" << solverType_ << "|" << endl;
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

bool preciceAdapter::AITS::AITSolver::readConfig(const YAML::Node adapterConfig)
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

    return true;
}

std::string preciceAdapter::AITS::AITSolver::determineSolverType()
{
    // NOTE: When coupling a different variable, you may want to
    // add more cases here. Or you may provide the solverType in the config.

    std::string solverType;

    // Determine the solver type: Compressible, Incompressible or Basic.
    // Look for the files transportProperties
    bool transportPropertiesExists = false;

    if (mesh_.foundObject<IOdictionary>(nameTransportProperties_))
    {
        transportPropertiesExists = true;
        DEBUG(adapterInfo("Found the transportProperties dictionary."));
    }
    else
    {
        DEBUG(adapterInfo("Did not find the transportProperties dictionary."));
    }

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

    return solverType;
}

void preciceAdapter::AITS::AITSolver::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added writer: Temperature."));
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

void preciceAdapter::AITS::AITSolver::addReaders(std::string dataName, Interface * interface)
{

    if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added reader: Temperature."));
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
