#include "Volume_Coupling.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::Volume_Coupling::Volume_Coupling::Volume_Coupling
(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime
)
:
mesh_(mesh),
runTime_(runTime)
{}

bool preciceAdapter::Volume_Coupling::Volume_Coupling::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the Volume_Coupling module..."));

    // Read the newcase-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    /* TODO: If we need different solver types,
    /  here is the place to determine it.
    */

    return true;
}

bool preciceAdapter::Volume_Coupling::Volume_Coupling::readConfig(const IOdictionary& adapterConfig)
{
    /* TODO: Read the solver type, if needed.
    /  If you want to determine it automatically, implement a method
    /  as in CHT/CHT.C
    */


    /* TODO: Read the names of any needed fields and parameters.
    * Include the force here?
    */
    const dictionary Volume_Couplingdict = adapterConfig.subOrEmptyDict("Volume_Coupling");

    // Read the name of the field Fluid_Velocity (if different)
    nameFluid_Velocity_ = Volume_Couplingdict.lookupOrDefault<word>("nameFluid_Velocity", "Fluid_Velocity");
    DEBUG(adapterInfo("    Fluid_Velocity field name : " + nameFluid_Velocity_));

    // Read the name of the field Volume_Porosity (if different)
    nameT_ = Htdict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    Temperature field name : " + nameT_));

    return true;
}

void preciceAdapter::Volume_Coupling::Volume_Coupling::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Fluid_Velocity") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Generic_volVectorField(mesh_, nameFluid_Velocity_)
        );
        DEBUG(adapterInfo("Added writer: Fluid_Velocity."));
    }

    else if (dataName.find("T") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Fluid_properties::Generic_volScalarField(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added writer: T."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
    //return true;
}

void preciceAdapter::Volume_Coupling::Volume_Coupling::addReaders(std::string dataName, Interface * interface)
{
    if (dataName.find("Fluid_Velocity") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Generic_volVectorField(mesh_, nameFluid_Velocity_)
        );
        DEBUG(adapterInfo("Added reader: Fluid_Velocity."));
    }

    else if (dataName.find("T") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Fluid_properties::Generic_volScalarField(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added reader: T."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
