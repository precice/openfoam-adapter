#include "KappaEffective_compressible.H"
#include "primitivePatchInterpolation.H"

#include "Utilities.H"

using namespace Foam;

//----- preciceAdapter::CHT::KappaEff_Compressible ------------------

preciceAdapter::CHT::KappaEff_Compressible::KappaEff_Compressible(
    const Foam::fvMesh& mesh)
: mesh_(mesh),
  turbulence_(
      mesh.time().db().lookupObject<compressible::turbulenceModel>("turbulenceModel"))
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
            const basicThermo& thermo = mesh_.time().db().lookupObject<basicThermo>("thermophysicalProperties");
             kappaEff_ = patchInterpolator.faceToPointInterpolate(turbulence_.alphaEff()().boundaryField()[patchID] * thermo.Cp()().boundaryField()[patchID]);
    }
    else
    {
            const basicThermo& thermo = mesh_.time().db().lookupObject<basicThermo>("thermophysicalProperties");
             kappaEff_ = turbulence_.alphaEff()().boundaryField()[patchID] * thermo.Cp()().boundaryField()[patchID];
    }
}

scalar preciceAdapter::CHT::KappaEff_Compressible::getAt(int i)
{
    return kappaEff_[i];
}


