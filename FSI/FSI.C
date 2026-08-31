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
        solverType_.compare("compressible") == 0
        || solverType_.compare("incompressible") == 0
        || solverType_.compare("solid") == 0)
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
        DEBUG(adapterInfo("Determining the solver type for the FSI module... (override by setting solverType to one of {compressible, incompressible, solid})"));
        solverType_ = determineSolverType();
    }

    return true;
}

bool preciceAdapter::FSI::FluidStructureInteraction::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary& FSIdict = adapterConfig.subOrEmptyDict("FSI");

    // Read the solver type (if not specified, it is determined automatically)
    solverType_ = FSIdict.lookupOrDefault<word>("solverType", "");
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    // When restarting FSI simulations, we may need to account for previous displacement.
    // We do this by resetting the displacement when defining the interface.
    // Since this is a feature that may not work as expected, depending on the implementation of the
    // structure solver used, we make this feature opt-in.
    restartFromDeformed_ = FSIdict.lookupOrDefault<bool>("restartFromDeformed", true);

    if (solverType_ == "solid" && !restartFromDeformed_)
    {
        adapterInfo("The option \"restartFromDeformed\" is only valid for Fluid solvers. Solid solvers usually use exclusively the reference configuration for computations and the mesh is not deforming over time.", "error");
    }

    if (solverType_ != "solid")
    {
        DEBUG(adapterInfo("    restart from deformed : " + std::to_string(restartFromDeformed_)));
    }
    /* TODO: Read the names of any needed fields and parameters.
     * Include the force here?
     */

    // Read the name of the pointDisplacement field (if different)
    namePointDisplacement_ = FSIdict.lookupOrDefault<word>("namePointDisplacement", "pointDisplacement");
    DEBUG(adapterInfo("    pointDisplacement field name : " + namePointDisplacement_));

    // Read the name of the pointDisplacement field (if different)
    nameCellDisplacement_ = FSIdict.lookupOrDefault<word>("nameCellDisplacement", "cellDisplacement");
    DEBUG(adapterInfo("    cellDisplacement field name : " + nameCellDisplacement_));

    // Read the name of the force field (if different)
    nameForce_ = FSIdict.lookupOrDefault<word>("nameForce", "Force");
    DEBUG(adapterInfo("    force field name : " + nameForce_));

    // Read the names of the propellerDisk fvModels to be coupled.
    // The order defines both the interface fixedPoints order and the
    // order in which the propeller loads are written to preCICE.
    // TODO: This is perhaps better placed in the interface dict.
    auto propellers = FSIdict.lookupOrDefault<wordList>("propellers", wordList());
    for (auto propeller : propellers)
    {
        propellerNames_.push_back(propeller);
        DEBUG(adapterInfo("    propeller fvModel : " + propeller));
    }

    // Read the control-surface definitions. Each entry is a sub-dictionary
    // with the surface patch name, the hinge point and the rotation axis:
    //   controlSurfaces ( { patch elevator; hinge (0.5 0.25 0.05); axis (1 0 0); } );
    // The order must match the fixedPoints of the Control-Mesh interface.
    if (FSIdict.found("controlSurfaces"))
    {
        const List<dictionary> surfList(FSIdict.lookup("controlSurfaces"));
        for (const dictionary& surf : surfList)
        {
            ControlSurfaceConfig cfg;
            cfg.patch = surf.lookup<word>("patch");
            const vector hinge = surf.lookup<vector>("hinge");
            const vector axis = surf.lookup<vector>("axis");
            cfg.hinge = {hinge.x(), hinge.y(), hinge.z()};
            cfg.axis = {axis.x(), axis.y(), axis.z()};
            controlSurfaces_.push_back(cfg);
            DEBUG(adapterInfo("    control surface : " + cfg.patch));
        }
    }

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
        const volScalarField& p_ = mesh_.lookupObject<volScalarField>("p");

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


bool preciceAdapter::FSI::FluidStructureInteraction::addWriters(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new Force(mesh_, solverType_, nameForce_) /* TODO: Add any other arguments here */
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
    else if (dataName.find("Thrust") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new PropellerLoad(mesh_, propellerNames_, false));
        DEBUG(adapterInfo("Added writer: PropellerLoad (thrust)."));
    }
    else if (dataName.find("PropTorque") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new PropellerLoad(mesh_, propellerNames_, true));
        DEBUG(adapterInfo("Added writer: PropellerLoad (torque)."));
    }
    else if (dataName.find("HingeForce") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new SurfaceHinge(mesh_, solverType_, controlSurfaces_, false));
        DEBUG(adapterInfo("Added writer: SurfaceHinge (force)."));
    }
    else if (dataName.find("HingeMoment") == 0)
    {
        interface->addCouplingDataWriter(
            dataName,
            new SurfaceHinge(mesh_, solverType_, controlSurfaces_, true));
        DEBUG(adapterInfo("Added writer: SurfaceHinge (moment)."));
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

bool preciceAdapter::FSI::FluidStructureInteraction::addReaders(std::string dataName, Interface* interface)
{
    bool found = true; // Set to false later, if needed.

    if (dataName.find("Force") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Force(mesh_, solverType_, nameForce_) /* TODO: Add any other arguments here */
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
    else if (dataName.find("AirVelocity") == 0 || dataName.find("Velocity") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new Velocity(mesh_));
        DEBUG(adapterInfo("Added reader: Velocity."));
    }
    else if (dataName.find("Deflection") == 0)
    {
        interface->addCouplingDataReader(
            dataName,
            new ControlDeflection(mesh_, controlSurfaces_));
        DEBUG(adapterInfo("Added reader: ControlDeflection."));
    }
    else
    {
        found = false;
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.

    return found;
}

std::string preciceAdapter::FSI::FluidStructureInteraction::getCellDisplacementFieldName()
{
    return nameCellDisplacement_;
}

std::string preciceAdapter::FSI::FluidStructureInteraction::getPointDisplacementFieldName()
{
    return namePointDisplacement_;
}

std::vector<std::string> preciceAdapter::FSI::FluidStructureInteraction::getPropellerNames()
{
    return propellerNames_;
}

std::vector<preciceAdapter::FSI::ControlSurfaceConfig> preciceAdapter::FSI::FluidStructureInteraction::getControlSurfaces()
{
    return controlSurfaces_;
}

bool preciceAdapter::FSI::FluidStructureInteraction::isRestartingFromDeformed()
{
    return restartFromDeformed_;
}
