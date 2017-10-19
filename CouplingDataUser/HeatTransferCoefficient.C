#include "HeatTransferCoefficient.H"

#include "fvCFD.H"
#include "mixedFvPatchFields.H"

using namespace Foam;

//----- preciceAdapter::User::HeatTransferCoefficient --------------------------

preciceAdapter::User::HeatTransferCoefficient::HeatTransferCoefficient
(
    const Foam::fvMesh& mesh
)
:
T_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>("T")
    )
),
mesh_(mesh)
{
    dataType_ = scalar;
}


void preciceAdapter::User::HeatTransferCoefficient::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        extractKappaEff(patchID);

        // TODO: The implementation is a bit different than before. Validate.
        // Get the face-cell distance coefficients on the patch
        const scalarField & delta = mesh_.boundary()[patchID].deltaCoeffs();

        // For all the cells on the patch
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


void preciceAdapter::User::HeatTransferCoefficient::read(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Extract the effective conductivity on the patch
        extractKappaEff(patchID);

        // TODO: The implementation is a bit different than before. Validate.
        // Get the face-cell distance coefficients on the patch
        const scalarField & delta = mesh_.boundary()[patchID].deltaCoeffs();

        // Get a reference to the temperature on the patch
        mixedFvPatchScalarField & TPatch =
            refCast<mixedFvPatchScalarField>(T_->boundaryFieldRef()[patchID]);

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


//----- preciceAdapter::User::HeatTransferCoefficient_Compressible -------------

preciceAdapter::User::
HeatTransferCoefficient_Compressible::HeatTransferCoefficient_Compressible
(
    const Foam::fvMesh& mesh
)
:
HeatTransferCoefficient(mesh),
Kappa_(new KappaEff_Compressible(mesh))
{
}

preciceAdapter::User::HeatTransferCoefficient_Compressible::
~HeatTransferCoefficient_Compressible()
{
    delete Kappa_;
}

void preciceAdapter::User::HeatTransferCoefficient_Compressible::
extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::User::HeatTransferCoefficient_Compressible::
getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

//----- preciceAdapter::User::HeatTransferCoefficient_Incompressible -----------

preciceAdapter::User::HeatTransferCoefficient_Incompressible::
HeatTransferCoefficient_Incompressible
(
    const Foam::fvMesh& mesh
)
:
HeatTransferCoefficient(mesh),
Kappa_(new KappaEff_Incompressible(mesh))
{
}

preciceAdapter::User::HeatTransferCoefficient_Incompressible::
~HeatTransferCoefficient_Incompressible()
{
    delete Kappa_;
}

void preciceAdapter::User::HeatTransferCoefficient_Incompressible::
extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::User::HeatTransferCoefficient_Incompressible::
getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}

//----- preciceAdapter::User::HeatTransferCoefficient_Basic -----------------------------------

preciceAdapter::User::HeatTransferCoefficient_Basic::
HeatTransferCoefficient_Basic
(
    const Foam::fvMesh& mesh
)
:
HeatTransferCoefficient(mesh),
Kappa_(new KappaEff_Basic(mesh))
{
}

preciceAdapter::User::HeatTransferCoefficient_Basic::
~HeatTransferCoefficient_Basic()
{
    delete Kappa_;
}

void preciceAdapter::User::HeatTransferCoefficient_Basic::
extractKappaEff(uint patchID)
{
    Kappa_->extract(patchID);
}

scalar preciceAdapter::User::HeatTransferCoefficient_Basic::
getKappaEffAt(int i)
{
    return Kappa_->getAt(i);
}
