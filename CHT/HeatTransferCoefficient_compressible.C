#include "HeatTransferCoefficient_compressible.H"

#include "fvCFD.H"
#include "mixedFvPatchFields.H"
#include "primitivePatchInterpolation.H"

using namespace Foam;

//----- preciceAdapter::CHT::HeatTransferCoefficient_Compressible -------------
preciceAdapter::CHT::
    HeatTransferCoefficient_Compressible::HeatTransferCoefficient_Compressible(
        const Foam::fvMesh& mesh,
        const std::string nameT)
: HeatTransferCoefficient(mesh, nameT),
  Kappa_(new KappaEff_Compressible(mesh))
{
}

preciceAdapter::CHT::HeatTransferCoefficient_Compressible::
    ~HeatTransferCoefficient_Compressible()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatTransferCoefficient_Compressible::
    extractKappaEff(uint patchID, bool meshConnectivity)
{
    Kappa_->extract(patchID, meshConnectivity);
}

scalar preciceAdapter::CHT::HeatTransferCoefficient_Compressible::
    getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

