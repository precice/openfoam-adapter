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

    /* TODO: If we need different solver types,
    /  here is the place to determine it.
    */

    return true;
}

bool preciceAdapter::FSI::FluidStructureInteraction::readConfig(const YAML::Node adapterConfig)
{
    /* TODO: Read the solver type, if needed.
    /  If you want to determine it automatically, implement a method
    /  as in CHT/CHT.C
    */

    /* TODO: Read the names of any needed fields and parameters.
    * Include the force here?
    */

    // Read the name of the pointDisplacement field (if different)
    if (adapterConfig["namePointDisplacement"])
    {
        namePointDisplacement_ = adapterConfig["namePointDisplacement"].as<std::string>();
    }
    DEBUG(adapterInfo("    pointDisplacement field name : " + namePointDisplacement_));

    return true;
}

void preciceAdapter::FSI::FluidStructureInteraction::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Force(mesh_, runTime_.timeName())
        );
        DEBUG(adapterInfo("Added writer: Force."));
    }
    else if (dataName.find("DisplacementDelta") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new DisplacementDelta(mesh_, namePointDisplacement_)
        );
        DEBUG(adapterInfo("Added writer: DisplacementDelta."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Displacement(mesh_, namePointDisplacement_)
        );
        DEBUG(adapterInfo("Added writer: Displacement."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::FSI::FluidStructureInteraction::addReaders(std::string dataName, Interface * interface)
{
    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Force(mesh_, runTime_.timeName())
        );
        DEBUG(adapterInfo("Added reader: Force."));
    }
    else if (dataName.find("DisplacementDelta") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new DisplacementDelta(mesh_, namePointDisplacement_)
        );
        DEBUG(adapterInfo("Added reader: DisplacementDelta."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Displacement(mesh_, namePointDisplacement_)
        );
        DEBUG(adapterInfo("Added reader: Displacement."));
    }
    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
