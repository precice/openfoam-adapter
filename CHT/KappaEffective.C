#include "KappaEffective.H"
#include "primitivePatchInterpolation.H"

#include "Utilities.H"

using namespace Foam;

//----- preciceAdapter::CHT::KappaEff_Compressible ------------------

preciceAdapter::CHT::KappaEff_Compressible::KappaEff_Compressible(
    const Foam::fvMesh& mesh)
: mesh_(mesh),
  thermophysicalTransportModel_(
      mesh.lookupObject<thermophysicalTransportModel>(thermophysicalTransportModel::typeName))
{
    DEBUG(adapterInfo("Constructed KappaEff_Compressible."));
}

void preciceAdapter::CHT::KappaEff_Compressible::extract(uint patchID, bool meshConnectivity)
{
    if (meshConnectivity)
    {
        //Create an Interpolation object at the boundary Field
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        //Interpolate kappaEff_ from centers to nodes
        kappaEff_ = patchInterpolator.faceToPointInterpolate(thermophysicalTransportModel_.kappaEff()().boundaryField()[patchID]);
    }
    else
    {
        // Extract kappaEff_ from the thermophysical transport model
        kappaEff_ = thermophysicalTransportModel_.kappaEff()().boundaryField()[patchID];
    }
}

scalar preciceAdapter::CHT::KappaEff_Compressible::getAt(int i)
{
    return kappaEff_[i];
}


//----- preciceAdapter::CHT::KappaEff_Incompressible ------------------

preciceAdapter::CHT::KappaEff_Incompressible::KappaEff_Incompressible(
    const Foam::fvMesh& mesh)
: mesh_(mesh),
  thermophysicalTransportModel_(
      mesh.lookupObject<thermophysicalTransportModel>(thermophysicalTransportModel::typeName))
{
    DEBUG(adapterInfo("Constructed KappaEff_Incompressible."));
}

void preciceAdapter::CHT::KappaEff_Incompressible::extract(uint patchID, bool meshConnectivity)
{
    if (meshConnectivity)
    {
        //Create an Interpolation object at the boundary Field
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        //Interpolate kappaEff_ from centers to nodes
        kappaEff_ = patchInterpolator.faceToPointInterpolate(thermophysicalTransportModel_.kappaEff()().boundaryField()[patchID]);
    }
    else
    {
        // Extract kappaEff_ from the thermophysical transport model
        kappaEff_ = thermophysicalTransportModel_.kappaEff()().boundaryField()[patchID];
    }
}

scalar preciceAdapter::CHT::KappaEff_Incompressible::getAt(int i)
{
    return kappaEff_[i];
}

//----- preciceAdapter::CHT::KappaEff_Basic ---------------------------

preciceAdapter::CHT::KappaEff_Basic::KappaEff_Basic(
    const Foam::fvMesh& mesh,
    const std::string nameKappa)
: mesh_(mesh),
  nameKappa_(nameKappa)
{
    DEBUG(adapterInfo("Constructed KappaEff_Basic."));
    DEBUG(adapterInfo("  Name of conductivity: " + nameKappa_));

    // Get the preciceDict/CHT dictionary
    const dictionary CHTDict =
        mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("CHT");

    // Read the conductivity
    if (!CHTDict.readIfPresent<dimensionedScalar>(nameKappa_, kappaEff_))
    {
        adapterInfo(
            "Cannot find the conductivity in preciceDict/CHT using the name " + nameKappa_,
            "error");
    }
    else
    {
        DEBUG(adapterInfo("k = " + std::to_string(kappaEff_.value())));
    }
}

void preciceAdapter::CHT::KappaEff_Basic::extract(uint patchID, bool meshConnectivity)
{
    // Already extracted in the constructor
}

scalar preciceAdapter::CHT::KappaEff_Basic::getAt(int i)
{
    // For a basic solver, the kappaEff is only one value.
    // Therefore, return the same value all the time.
    // This is done so that the same write() and read() can be used
    // for all the subclasses of HeatFlux (or other coupling data users).
    return kappaEff_.value();
}
