#include "HeatTransferCoefficient_incompressible.H"

#include "fvCFD.H"
#include "mixedFvPatchFields.H"
#include "primitivePatchInterpolation.H"

using namespace Foam;

//----- preciceAdapter::CHT::HeatTransferCoefficient_Incompressible -----------
preciceAdapter::CHT::HeatTransferCoefficient_Incompressible::
    HeatTransferCoefficient_Incompressible(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameRho,
        const std::string nameCp,
        const std::string namePr,
        const std::string nameAlphat)
: HeatTransferCoefficient(mesh, nameT),
  Kappa_(new KappaEff_Incompressible(mesh, nameRho, nameCp, namePr, nameAlphat))
{
}

preciceAdapter::CHT::HeatTransferCoefficient_Incompressible::
    ~HeatTransferCoefficient_Incompressible()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatTransferCoefficient_Incompressible::
    extractKappaEff(uint patchID, bool meshConnectivity)
{
    Kappa_->extract(patchID, meshConnectivity);
}

scalar preciceAdapter::CHT::HeatTransferCoefficient_Incompressible::
    getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

