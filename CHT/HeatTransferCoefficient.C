#include "HeatTransferCoefficient.H"

#include "fvCFD.H"
#include "mixedFvPatchFields.H"
#include "primitivePatchInterpolation.H"

using namespace Foam;

//----- preciceAdapter::CHT::HeatTransferCoefficient --------------------------

preciceAdapter::CHT::HeatTransferCoefficient::HeatTransferCoefficient(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}


void preciceAdapter::CHT::HeatTransferCoefficient::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        extractKappaEff(patchID, meshConnectivity);

        // Get the face-cell distance coefficients on the patch
        const scalarField& delta(
            mesh_.boundary()[patchID].deltaCoeffs());

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if (meshConnectivity)
        {
            //Setup Interpolation object
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            scalarField deltaPoints;

            //Interpolate
            deltaPoints = patchInterpolator.faceToPointInterpolate(delta);

            // For all the cells on the patch
            forAll(deltaPoints, i)
            {
                // Fill the buffer with the values kappaEff * delta.
                // Kappa is not precomputed, in order to be able to use the
                // same write() method also for basic solvers, where
                // kappaEff is not a scalarField.
                buffer[bufferIndex++] = getKappaEffAt(i) * deltaPoints[i];
            }
        }
        else
        {
            forAll(delta, i)
            {
                // Fill the buffer with the values kappaEff * delta.
                // Kappa is not precomputed, in order to be able to use the
                // same write() method also for basic solvers, where
                // kappaEff is not a scalarField.
                buffer[bufferIndex++] = getKappaEffAt(i) * delta[i];
            }
        }
    }
}


void preciceAdapter::CHT::HeatTransferCoefficient::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        // TODO: At the moment, reading with connectivity is not supported
        extractKappaEff(patchID, /*meshConnectivity=*/false);

        // Get the face-cell distance coefficients on the patch
        const scalarField& delta(
            mesh_.boundary()[patchID].deltaCoeffs());

        // Get a reference to the temperature on the patch
        mixedFvPatchScalarField& TPatch(
            refCast<mixedFvPatchScalarField>(
                T_->boundaryFieldRef()[patchID]));

        // For every cell on the patch
        forAll(TPatch, i)
        {
            // Compute the value of kappaEff * delta on this side
            // of the interface.
            // Kappa is not precomputed, in order to be able to use the
            // same read() method also for basic solvers, where
            // kappaEff is not a scalarField.
            double myKappaDelta = getKappaEffAt(i) * delta[i];

            // Get the value of KappaEff * delta from the other side
            // of the interface.
            double neighborKappaDelta = buffer[bufferIndex++];

            // Set the fraction (0-1) of value for the mixed boundary condition
            TPatch.valueFraction()[i] =
                neighborKappaDelta / (myKappaDelta + neighborKappaDelta);
        }
    }
}


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

//----- preciceAdapter::CHT::HeatTransferCoefficient_Basic -----------------------------------

preciceAdapter::CHT::HeatTransferCoefficient_Basic::
    HeatTransferCoefficient_Basic(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameKappa)
: HeatTransferCoefficient(mesh, nameT),
  Kappa_(new KappaEff_Basic(mesh, nameKappa))
{
}

preciceAdapter::CHT::HeatTransferCoefficient_Basic::
    ~HeatTransferCoefficient_Basic()
{
    delete Kappa_;
}

void preciceAdapter::CHT::HeatTransferCoefficient_Basic::
    extractKappaEff(uint patchID, bool meshConnectivity)
{
    Kappa_->extract(patchID, meshConnectivity);
}

scalar preciceAdapter::CHT::HeatTransferCoefficient_Basic::
    getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}
