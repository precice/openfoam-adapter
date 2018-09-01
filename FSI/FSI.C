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
    */

           // Read the name of the pointDisplacement field (if different)
    if (adapterConfig["namePointDisplacement"])
    {
        namePointDisplacement_ = adapterConfig["namePointDisplacement"].as<std::string>();
    }
    DEBUG(adapterInfo("    pointDisplacement field name : " + namePointDisplacement_));


           // Read the name of the velocity field (if different)
    if (adapterConfig["nameU"])
    {
        nameU_ = adapterConfig["nameU"].as<std::string>();
    }
    DEBUG(adapterInfo("    Velocity field name : " + nameU_));



    return true;
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
            new Force(mesh_, runTime_.timeName()) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added writer: Force."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            // TODO: Hard-coded number of locations! Fix!!!
            // new Displacement(mesh_, runTime_, 162) /* TODO: Add any other arguments here */
            new Displacement(mesh_, namePointDisplacement_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added writer: Displacement."));
    }
    // else if (dataName.find("Velocity") == 0)
    // {
    //     interface->addCouplingDataWriter
    //     (
    //         dataName,
    //         new Force(mesh_, nameVelocity_) /* TODO: Add any other arguments here */
    //     );
    //     DEBUG(adapterInfo("Added writer: Velocity."));
    // }
    

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
    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Force(mesh_, runTime_.timeName()) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Force."));
    }
    else if (dataName.find("Displacement") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            // TODO: Hard-coded number of locations! Fix!!!
            // new Displacement(mesh_, runTime_, 162) /* TODO: Add any other arguments here */
            new Displacement(mesh_, namePointDisplacement_) /* TODO: Add any other arguments here */
        );
        DEBUG(adapterInfo("Added reader: Displacement."));
    }
    // else if (dataName.find("Velocity") == 0)
    // {
    //     interface->addCouplingDataReader
    //     (
    //         dataName,
    //         // TODO: Hard-coded number of locations! Fix!!!
    //         // new Displacement(mesh_, runTime_, 162) /* TODO: Add any other arguments here */
    //         new Displacement(mesh_, nameVelocity_) /* TODO: Add any other arguments here */
    //     );
    //     DEBUG(adapterInfo("Added reader: Velocity."));
    // }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
