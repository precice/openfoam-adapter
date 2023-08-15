#include "HeatFlux_incompressible.H"
#include "primitivePatchInterpolation.H"

#include "fvCFD.H"

using namespace Foam;


//----- preciceAdapter::CHT::HeatFlux_Incompressible --------------------------
preciceAdapter::CHT::HeatFlux_Incompressible::HeatFlux_Incompressible(
    const Foam::fvMesh& mesh,
    const std::string nameT,
    const std::string nameRho,
    const std::string nameCp,
    const std::string namePr,
    const std::string nameAlphat)
: HeatFlux(mesh, nameT),
  Kappa_(new KappaEff_Incompressible(mesh, nameRho, nameCp, namePr, nameAlphat))
{
}

preciceAdapter::CHT::HeatFlux_Incompressible::~HeatFlux_Incompressible()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatFlux_Incompressible::extractKappaEff(uint patchID, bool meshConnectivity)
{
    Kappa_->extract(patchID, meshConnectivity);
}

scalar preciceAdapter::CHT::HeatFlux_Incompressible::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

