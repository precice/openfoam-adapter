#include "KappaEffective.H"

using namespace Foam;

//----- preciceAdapter::CHT::KappaEff_Compressible ------------------

preciceAdapter::CHT::KappaEff_Compressible::KappaEff_Compressible
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh),
turbulence_(
    mesh.lookupObject<compressible::turbulenceModel>(turbulenceModel::propertiesName)
)
{}

void preciceAdapter::CHT::KappaEff_Compressible::extract(uint patchID)
{
    // Extract kappaEff_ from the turbulence model
    kappaEff_ = turbulence_.kappaEff() ().boundaryField()[patchID];
}

scalar preciceAdapter::CHT::KappaEff_Compressible::getAt(int i)
{
    return kappaEff_[i];
}


//----- preciceAdapter::CHT::KappaEff_Incompressible ------------------

preciceAdapter::CHT::KappaEff_Incompressible::KappaEff_Incompressible
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh),
turbulence_(
    mesh.lookupObject<incompressible::turbulenceModel>(turbulenceModel::propertiesName)
)
{}

void preciceAdapter::CHT::KappaEff_Incompressible::extract(uint patchID)
{
    // Compute kappaEff_ from the turbulence model, using alpha and Prandl

    // Get the laminar viscosity from the turbulence model
    const scalarField & nu = turbulence_.nu() ().boundaryField()[patchID];

    // Make sure that the transportProperties exists.
    if (!mesh_.foundObject<IOdictionary>("transportProperties"))
    {
        FatalErrorInFunction
            << "The transportProperties dictionary needs "
            << "to exist and to contain Pr, rho, and Cp."
            << exit(FatalError);
    }

    // Get the transportProperties dictionary
    const dictionary & transportProperties =
        &mesh_.lookupObject<IOdictionary>("transportProperties");

    // Get the Prandl number from the transportProperties.
    // If it does not exist, an error is thrown automatically.
    const scalar & Pr =
        transportProperties.lookupType<dimensionedScalar>("Pr").value();

    // Compute the effective thermal diffusivity
    // (alphaEff = alpha + alphat = nu / Pr + nut / Prt)

    scalarField alphaEff;

    // Does the turbulent thermal diffusivity exist in the object registry?
    if (mesh_.foundObject<volScalarField>("alphat"))
    {
        const scalarField & alphat =
            mesh_.lookupObject<volScalarField>("alphat").boundaryField()[patchID];

        alphaEff = nu / Pr + alphat;
    }
    else
    {
        WarningInFunction
            << "The object alphat does not exist. "
            << "An incompressible solver should create it explicitly "
            << "(e.g. in the createFields.H)."
            << "Assuming only the laminar part of the thermal diffusivity."
            << nl;

        alphaEff = nu / Pr;
    }

    // Get the density from the transportProperties (must be provided and read).
    // If it does not exist, an error is thrown automatically.
    const scalar & rho =
        transportProperties.lookupType<dimensionedScalar>("rho").value();

    // Get the specific heat capacity from the transportProperties
    // (must be provided and read).
    // If it does not exist, an error is thrown automatically.
    const scalar & Cp =
        transportProperties.lookupType<dimensionedScalar>("Cp").value();

    // Compute the effective thermal conductivity
    kappaEff_ = alphaEff * rho * Cp;

}

scalar preciceAdapter::CHT::KappaEff_Incompressible::getAt(int i)
{
    return kappaEff_[i];
}

//----- preciceAdapter::CHT::KappaEff_Basic ---------------------------

preciceAdapter::CHT::KappaEff_Basic::KappaEff_Basic
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

void preciceAdapter::CHT::KappaEff_Basic::extract(uint patchID)
{
    // Extract kappaEff_ as a parameter from the transportProperties file

    // Make sure that the transportProperties exists.
    if (!mesh_.foundObject<IOdictionary>("transportProperties"))
    {
        FatalErrorInFunction
            << "The transportProperties dictionary needs "
            << "to exist and to contain k."
            << exit(FatalError);
    }

    // Get the transportProperties dictionary
    const dictionary& transportProperties =
        mesh_.lookupObject<IOdictionary>("transportProperties");

    // Get the conductivity from the file
    kappaEff_ = transportProperties.lookupType<dimensionedScalar>("k").value();
}

scalar preciceAdapter::CHT::KappaEff_Basic::getAt(int i)
{
    // For a basic solver, the kappaEff is only one value.
    // Therefore, return the same value all the time.
    // This is done so that the same write() and read() can be used
    // for all the subclasses of HeatFlux (or other coupling data users).
    return kappaEff_;
}
