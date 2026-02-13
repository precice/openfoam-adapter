#include "Generic.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::Generic::GenericInterface::GenericInterface(
    const Foam::fvMesh& mesh)
: mesh_(mesh) {}

bool preciceAdapter::Generic::GenericInterface::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the Generic module..."));

    // Scan OpenFOAM object registry once for all
    // available volScalarFields and volVectorFields
    for (const auto& solver_name : mesh_.sortedNames<volScalarField>())
    {
        availableVolScalarFields += solver_name + " ";
    }
    DEBUG(adapterInfo("    Available volScalarFields: " + availableVolScalarFields));

    for (const auto& solver_name : mesh_.sortedNames<volVectorField>())
    {
        availableVolVectorFields += solver_name + " ";
    }
    DEBUG(adapterInfo("    Available volVectorFields: " + availableVolVectorFields));

    // Read the Generic-module specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }

    return true;
}

bool preciceAdapter::Generic::GenericInterface::readConfig(const IOdictionary& adapterConfig)
{
    // Empty for now. No other specific configuration options for the Generic module.
    return true;
}

bool preciceAdapter::Generic::GenericInterface::addWriters(const preciceAdapter::FieldConfig& FieldConfig, Interface* interface)
{
    bool found = false;

    // Force to use the new schema with the Generic module
    if (FieldConfig.solver_name != "Undefined (legacy mode)")
    {
        // Determine type of field
        if (mesh_.foundObject<volScalarField>(FieldConfig.solver_name))
        {
            found = true;
            interface->addCouplingDataWriter(
                FieldConfig.name,
                new ScalarFieldCoupler(mesh_, FieldConfig));
        }
        else if (mesh_.foundObject<volVectorField>(FieldConfig.solver_name))
        {
            found = true;
            interface->addCouplingDataWriter(
                FieldConfig.name,
                new VectorFieldCoupler(mesh_, FieldConfig));
        }
        else
        {
            found = false;
            std::string msg = "Generic module: Data \"" + FieldConfig.name + "\", solver name: \"" + FieldConfig.solver_name + "\" not found!\n";
            msg += "Available fields: " + availableVolScalarFields + availableVolVectorFields;
            adapterInfo(msg, "warning");
        }
    }

    if (found)
    {
        DEBUG(adapterInfo("Added writer: " + FieldConfig.name));
    }
    return found;
}

bool preciceAdapter::Generic::GenericInterface::addReaders(const preciceAdapter::FieldConfig& FieldConfig, Interface* interface)
{
    bool found = false;

    // Force to use the new schema with the Generic modul
    if (FieldConfig.solver_name != "Undefined (legacy mode)")
    {
        // Determine type of field
        if (mesh_.foundObject<volScalarField>(FieldConfig.solver_name))
        {
            found = true;
            interface->addCouplingDataReader(
                FieldConfig.name,
                new ScalarFieldCoupler(mesh_, FieldConfig));
        }
        else if (mesh_.foundObject<volVectorField>(FieldConfig.solver_name))
        {
            found = true;
            interface->addCouplingDataReader(
                FieldConfig.name,
                new VectorFieldCoupler(mesh_, FieldConfig));
        }
        else
        {
            found = false;
            std::string msg = "Generic module: Data \"" + FieldConfig.name + "\" (solver name: \"" + FieldConfig.solver_name + "\") not found!\n";
            msg += "Available fields: " + availableVolScalarFields + availableVolVectorFields;
            adapterInfo(msg, "warning");
        }
    }

    if (found)
    {
        DEBUG(adapterInfo("Added reader: " + FieldConfig.name));
    }
    return found;
}
