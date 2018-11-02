#include "FSI.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::FSI::FluidStructureInteraction::FluidStructureInteraction
(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime
)
:
mesh_(mesh),
runTime_(runTime)
{}

bool preciceAdapter::FSI::FluidStructureInteraction::configure(const YAML::Node adapterConfig)
{
    DEBUG(adapterInfo("Configuring the FSI module..."));

    // Read the FSI-specific options from the adapter's configuration file
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

bool preciceAdapter::FSI::FluidStructureInteraction::readConfig(const YAML::Node adapterConfig)
{
    // Read the solver type (if not specified, it is determined automatically)
    if (adapterConfig["solverType"])
    {
        solverType_ = adapterConfig["solverType"].as<std::string>();
    }
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    /* TODO: Read the names of any needed fields and parameters.
    * Include the force here?
    */

    // Read the name of the pointDisplacement field (if different)
    if (adapterConfig["namePointDisplacement"])
    {
        namePointDisplacement_ = adapterConfig["namePointDisplacement"].as<std::string>();
    }
    DEBUG(adapterInfo("    pointDisplacement field name : " + namePointDisplacement_));


    // Read the name of the velocity field (if different)
    if (adapterConfig["nameVelocity"])
    {
        nameVelocity_ = adapterConfig["nameVelocity"].as<std::string>();
    }
    DEBUG(adapterInfo("    Velocity field name : " + nameVelocity_));

    return true;
}

std::string preciceAdapter::FSI::FluidStructureInteraction::determineSolverType()
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


void preciceAdapter::FSI::FluidStructureInteraction::addWriters(std::string dataName, Interface * interface)
{
    /* TODO: Add writers. See CHT/CHT.C for reference.
    /  We probably need to do this for displacements and forces.
    /  If different coupling data users per solver type are defined,
    /  we need to check for that here.
    */
    if (dataName.find("Force") == 0)
    {        
            interface->addCouplingDataWriter
            (
                dataName,
                new Force(mesh_, runTime_.timeName(), solverType_) /* TODO: Add any other arguments here */
            );
            DEBUG(adapterInfo("Added writer: Force for compressible solvers."));        
    }
    // TODO Do we need to include the displacement and velocity? They will never be written...
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Displacement(mesh_, namePointDisplacement_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added writer: Displacement."));
    }
    // TODO perform a similar strategy here as in the
    else if (dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Velocity(mesh_, runTime_, nameVelocity_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added writer: Velocity."));
    }


    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::FSI::FluidStructureInteraction::addReaders(std::string dataName, Interface * interface)
{
    /* TODO: Add readers. See CHT/CHT.C for reference.
    /  We probably need to do this for displacements and forces.
    /  If different coupling data users per solver type are defined,
    /  we need to check for that here.
    */

    // TODO do we need to include the force here, since it will not be read by openFOAM?
    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Force(mesh_, runTime_.timeName(), solverType_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Force."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Displacement(mesh_, namePointDisplacement_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Displacement."));
    //
    // TODO evaluate this.
    // The velocity is not in the dataNames, because it is not exchanged. In the case a displacement mesh
    // motion solver is used, it needs to be created, therefore it is listed in the same if-statement.
        interface->addCouplingDataReader
        (
            dataName,
            new Velocity(mesh_, runTime_, nameVelocity_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Velocity."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
