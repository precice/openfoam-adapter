#include "FSI.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::FSI::FluidStructureInteraction::FluidStructureInteraction(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime)
: mesh_(mesh),
  runTime_(runTime)
{
}

bool preciceAdapter::FSI::FluidStructureInteraction::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the FSI module..."));

    // Read the FSI-specific options from the adapter's configuration file
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
        DEBUG(adapterInfo("Determining the solver type for the FSI module... (override by setting solverType to one of {compressible, incompressible})"));
        solverType_ = determineSolverType();
    }

    return true;
}

bool preciceAdapter::FSI::FluidStructureInteraction::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary FSIdict = adapterConfig.subOrEmptyDict("FSI");

    // Read the solver type (if not specified, it is determined automatically)
    solverType_ = FSIdict.lookupOrDefault<word>("solverType", "");
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    /* TODO: Read the names of any needed fields and parameters.
    * Include the force here?
    */

    // Read the name of the pointDisplacement field (if different)
    namePointDisplacement_ = FSIdict.lookupOrDefault<word>("namePointDisplacement", "pointDisplacement");
    DEBUG(adapterInfo("    pointDisplacement field name : " + namePointDisplacement_));

    // Read the name of the pointDisplacement field (if different)
    nameCellDisplacement_ = FSIdict.lookupOrDefault<word>("nameCellDisplacement", "cellDisplacement");
    DEBUG(adapterInfo("    cellDisplacement field name : " + nameCellDisplacement_));

    return true;
}

// NOTE: This is exactly the same as in the CHT module.
std::string preciceAdapter::FSI::FluidStructureInteraction::determineSolverType()
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
                    "Please specify your solver type in the FSI section of the "
                    "preciceDict. Known solver types for FSI are: "
                    "incompressible and "
                    "compressible",
                    "error");
    }

    DEBUG(adapterInfo("Automatically determined solver type : " + solverType));

    return solverType;
}


void preciceAdapter::FSI::FluidStructureInteraction::addWriters(std::string dataName, Interface* interface)
{
    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Force(mesh_, solverType_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added writer: Force."));
    }
    else if (dataName.find("DisplacementDelta") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new DisplacementDelta(mesh_, namePointDisplacement_, nameCellDisplacement_));
        DEBUG(adapterInfo("Added writer: DisplacementDelta."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Displacement(mesh_, namePointDisplacement_, nameCellDisplacement_));
        DEBUG(adapterInfo("Added writer: Displacement."));
    }
    else if (dataName.find("Stress") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Stress(mesh_, solverType_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added writer: Stress."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::FSI::FluidStructureInteraction::addReaders(std::string dataName, Interface* interface)
{
    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Force(mesh_, solverType_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Force."));
    }
    else if (dataName.find("DisplacementDelta") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new DisplacementDelta(mesh_, namePointDisplacement_, nameCellDisplacement_));
        DEBUG(adapterInfo("Added reader: DisplacementDelta."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Displacement(mesh_, namePointDisplacement_, nameCellDisplacement_));
        DEBUG(adapterInfo("Added reader: Displacement."));
    }
    else if (dataName.find("Stress") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Stress(mesh_, solverType_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Stress."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
