#include "Momentum.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::Momentum::Momentum::Momentum
(
    const Foam::fvMesh& mesh,
    const Foam::Time& runTime
)
:
mesh_(mesh),
runTime_(runTime)
{}

bool preciceAdapter::Momentum::Momentum::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the Momentum module..."));

    // Read the newcase-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig)) return false;

    /* TODO: If we need different solver types,
    /  here is the place to determine it.
    */

    return true;
}

bool preciceAdapter::Momentum::Momentum::readConfig(const IOdictionary& adapterConfig)
{
    /* TODO: Read the solver type, if needed.
    /  If you want to determine it automatically, implement a method
    /  as in CHT/CHT.C
    */


    /* TODO: Read the names of any needed fields and parameters.
    * Include the force here?
    */
    const dictionary Momentumdict = adapterConfig.subOrEmptyDict("MOMENTUM");

    // Read the name of the field Your_Volume_Field (if different)
    nameYour_Volume_Field_ = Momentumdict.lookupOrDefault<word>("nameYour_Volume_Field", "Your_Volume_Field");
    DEBUG(adapterInfo("    Your_Volume_Field field name : " + nameYour_Volume_Field_));

    return true;
}

void preciceAdapter::Momentum::Momentum::addWriters(std::string dataName, Interface * interface)
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

void preciceAdapter::Momentum::Momentum::addReaders(std::string dataName, Interface * interface)
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
