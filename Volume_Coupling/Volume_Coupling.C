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

    // Read the name of the field Your_Volume_Field (if different)
    nameYour_Volume_Field_ = Volume_Couplingdict.lookupOrDefault<word>("nameYour_Volume_Field", "Your_Volume_Field");
    DEBUG(adapterInfo("    Your_Volume_Field field name : " + nameYour_Volume_Field_));

    return true;
}

void preciceAdapter::Volume_Coupling::Volume_Coupling::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Your_Volume_Field") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Generic_volScalarField(mesh_, nameYour_Volume_Field_)
        );
        DEBUG(adapterInfo("Added writer: Your_Volume_Field."));
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
    if (dataName.find("Your_Volume_Field") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Generic_volScalarField(mesh_, nameYour_Volume_Field_)
        );
        DEBUG(adapterInfo("Added reader: Your_Volume_Field."));
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
