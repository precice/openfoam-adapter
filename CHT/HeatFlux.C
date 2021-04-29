#include "HeatFlux.H"
#include "primitivePatchInterpolation.H"

#include "fvCFD.H"

using namespace Foam;

//----- preciceAdapter::CHT::HeatFlux -----------------------------------------

preciceAdapter::CHT::HeatFlux::HeatFlux(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}

void preciceAdapter::CHT::HeatFlux::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const scalarField gradientPatch(
            refCast<const fixedValueFvPatchScalarField>(
                T_->boundaryField()[patchID])
                .snGrad());

        // Extract the effective conductivity on the patch
        extractKappaEff(patchID, meshConnectivity);

        // If we use the mesh connectivity, we interpolate from the centres to the nodes
        if (meshConnectivity)
        {
            //Setup Interpolation object
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            scalarField gradientPoints;

            //Interpolate
            gradientPoints = patchInterpolator.faceToPointInterpolate(gradientPatch);

            // For every cell of the patch
            forAll(gradientPoints, i)
            {
                // Copy the heat flux into the buffer
                // Q = - k * gradient(T)
                //TODO: Interpolate kappa in case of a turbulent calculation
                buffer[bufferIndex++] =
                    -getKappaEffAt(i) * gradientPoints[i];
            }
        }
        else
        {
            // For every cell of the patch
            forAll(gradientPatch, i)
            {
                // Copy the heat flux into the buffer
                // Q = - k * gradient(T)
                //TODO: Interpolate kappa in case of a turbulent calculation
                buffer[bufferIndex++] =
                    -getKappaEffAt(i) * gradientPatch[i];
            }
        }
    }
}

void preciceAdapter::CHT::HeatFlux::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        // TODO: At the moment, reading with connectivity is not supported
        extractKappaEff(patchID, /*meshConnectivity=*/false);

        // Get the temperature gradient boundary patch
        scalarField& gradientPatch(
            refCast<fixedGradientFvPatchScalarField>(
                T_->boundaryFieldRef()[patchID])
                .gradient());

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Compute and assign the gradient from the buffer.
            // The sign of the heat flux needs to be inversed,
            // as the buffer contains the flux that enters the boundary:
            // gradient(T) = -Q / -k
            gradientPatch[i] =
                buffer[bufferIndex++] / getKappaEffAt(i);
        }
    }
}

//----- preciceAdapter::CHT::HeatFlux_Compressible ----------------------------

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

//----- preciceAdapter::CHT::HeatFlux_Basic -----------------------------------

preciceAdapter::CHT::HeatFlux_Basic::HeatFlux_Basic(
    const Foam::fvMesh& mesh,
    const std::string nameT,
    const std::string nameKappa)
: HeatFlux(mesh, nameT),
  Kappa_(new KappaEff_Basic(mesh, nameKappa))
{
}

preciceAdapter::CHT::HeatFlux_Basic::~HeatFlux_Basic()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatFlux_Basic::extractKappaEff(uint patchID, bool meshConnectivity)
{
    Kappa_->extract(patchID, meshConnectivity);
}

scalar preciceAdapter::CHT::HeatFlux_Basic::getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}
