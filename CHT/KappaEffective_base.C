#include "KappaEffective_base.H"
#include "primitivePatchInterpolation.H"

#include "Utilities.H"

using namespace Foam;

//----- preciceAdapter::CHT::KappaEff_Basic ---------------------------

preciceAdapter::CHT::KappaEff_Basic::KappaEff_Basic(
    const Foam::fvMesh& mesh,
    const std::string nameKappa)
: mesh_(mesh),
  nameKappa_(nameKappa),
  kappaEff_(0)
{
    DEBUG(adapterInfo("Constructed KappaEff_Basic."));
    DEBUG(adapterInfo("  Name of conductivity: " + nameKappa_));

    // Get the preciceDict/CHT dictionary
    const dictionary& CHTDict =
        mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("CHT");

    // Read the conductivity
    if (!CHTDict.readIfPresent(nameKappa_, kappaEff_))
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
