#include "HeatFlux_compressible.H"
#include "primitivePatchInterpolation.H"

#include "fvCFD.H"

using namespace Foam;

preciceAdapter::CHT::HeatFlux_Compressible::HeatFlux_Compressible(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: HeatFlux(mesh, nameT),
  Kappa_(new KappaEff_Compressible(mesh))
{
}

preciceAdapter::CHT::HeatFlux_Compressible::~HeatFlux_Compressible()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatFlux_Compressible::extractKappaEff(uint patchID, bool meshConnectivity)
{
    Kappa_->extract(patchID, meshConnectivity);
}

scalar preciceAdapter::CHT::HeatFlux_Compressible::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

