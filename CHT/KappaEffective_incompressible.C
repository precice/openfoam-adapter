#include "KappaEffective_incompressible.H"
#include "primitivePatchInterpolation.H"

#include "Utilities.H"

using namespace Foam;

//----- preciceAdapter::CHT::KappaEff_Incompressible ------------------
preciceAdapter::CHT::KappaEff_Incompressible::KappaEff_Incompressible(
    const Foam::fvMesh& mesh,
    const std::string nameRho,
    const std::string nameCp,
    const std::string namePr,
    const std::string nameAlphat)
: mesh_(mesh),
  turbulence_(
      mesh.time().db().lookupObject<incompressible::turbulenceModel>("turbulenceModel")),
  nameRho_(nameRho),
  nameCp_(nameCp),
  namePr_(namePr),
  nameAlphat_(nameAlphat),
  Pr_(0),
  rho_(0),
  Cp_(0)
{
    DEBUG(adapterInfo("Constructed KappaEff_Incompressible."));
    DEBUG(adapterInfo("  Name of density: " + nameRho_));
    DEBUG(adapterInfo("  Name of heat capacity: " + nameCp_));
    DEBUG(adapterInfo("  Name of Prandl number: " + namePr_));
    DEBUG(adapterInfo("  Name of turbulent thermal diffusivity: " + nameAlphat_));

    // Get the preciceDict/CHT dictionary
    const dictionary& CHTDict =
        mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("CHT");

    // Read the Prandtl number
    if (!CHTDict.readIfPresent(namePr_, Pr_))
    {
        adapterInfo(
            "Cannot find the Prandtl number in preciceDict/CHT using the name " + namePr_,
            "error");
    }
    else
    {
        DEBUG(adapterInfo("  Pr = " + std::to_string(Pr_.value())));
    }

    // Read the density
    if (!CHTDict.readIfPresent(nameRho_, rho_))
    {
        adapterInfo(
            "Cannot find the density in preciceDict/CHT using the name " + nameRho_,
            "error");
    }
    else
    {
        DEBUG(adapterInfo("  rho = " + std::to_string(rho_.value())));
    }

    // Read the heat capacity
    if (!CHTDict.readIfPresent(nameCp_, Cp_))
    {
        adapterInfo(
            "Cannot find the heat capacity in preciceDict/CHT using the name " + nameCp_,
            "error");
    }
    else
    {
        DEBUG(adapterInfo("  Cp = " + std::to_string(Cp_.value())));
    }
}

void preciceAdapter::CHT::KappaEff_Incompressible::extract(uint patchID, bool meshConnectivity)
{
    // Compute kappaEff_ from the turbulence model, using alpha and Prandl

    // Get the laminar viscosity from the turbulence model
    // TODO: Do we really need turbulence at the end?
    const scalarField& nu(
        turbulence_.nu().boundaryField()[patchID]);

    // Compute the effective thermal diffusivity
    // (alphaEff = alpha + alphat = nu / Pr + nut / Prt)

    scalarField alphaEff;

    // Does the turbulent thermal diffusivity exist in the object registry?
    if (mesh_.foundObject<volScalarField>(nameAlphat_))
    {
        const scalarField& alphat =
            mesh_.lookupObject<volScalarField>(nameAlphat_).boundaryField()[patchID];

        alphaEff = nu / Pr_.value() + alphat;
    }
    else
    {
        WarningInFunction
            << "The object alphat does not exist. "
            << "An incompressible solver should create it explicitly "
            << "(e.g. in the createFields.H)."
            << "Assuming only the laminar part of the thermal diffusivity."
            << nl;

        alphaEff = nu / Pr_.value();
    }

    // Compute the effective thermal conductivity and store it in a temp variable
    scalarField kappaEff_temp(
        alphaEff * rho_.value() * Cp_.value());

    if (meshConnectivity)
    {
        //Create an Interpolation object at the boundary Field
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        //Interpolate kappaEff_ from centers to nodes, if desired
        kappaEff_ = patchInterpolator.faceToPointInterpolate(kappaEff_temp);
    }
    else
    {
        // if no interpolation
        kappaEff_ = kappaEff_temp;
    }
}

scalar preciceAdapter::CHT::KappaEff_Incompressible::getAt(int i)
{
    return kappaEff_[i];
}

