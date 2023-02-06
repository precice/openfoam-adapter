#include "Velocity.H"
#include "coupledVelocityFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::Velocity::Velocity(
    const Foam::fvMesh& mesh,
    const std::string nameU)
: U_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(nameU))),
  phi_(const_cast<surfaceScalarField*>(
      &mesh.lookupObject<surfaceScalarField>("phi")))
{
    dataType_ = vector;
}

void preciceAdapter::FF::Velocity::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Correct the velocity by the boundary face flux
        scalarField phip = phi_->boundaryFieldRef()[patchID];
        vectorField n = U_->boundaryField()[patchID].patch().nf();
        scalarField magS = U_->boundaryField()[patchID].patch().magSf();
        vectorField u_corrected = U_->boundaryField()[patchID];
        //  - n*(n & U_->boundaryField()[patchID]) + n*phip/magS;

        // Info << endl << "Subtract: " << n[440]*(n[440]&U_->boundaryField()[patchID][440]) << endl;
        // Info << "Add: " << n[440]*phip[440]/magS[440] << endl;
        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++] =
                u_corrected[i].x();

            // y-dimension
            buffer[bufferIndex++] =
                u_corrected[i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    u_corrected[i].z();
            }
        }
    }
}

void preciceAdapter::FF::Velocity::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity value boundary patch
        vectorField* valuePatchPtr = &U_->boundaryFieldRef()[patchID];
        if (isA<coupledVelocityFvPatchField>(U_->boundaryFieldRef()[patchID]))
        {
            valuePatchPtr = &refCast<coupledVelocityFvPatchField>(
                                 U_->boundaryFieldRef()[patchID])
                                 .refValue();
        }
        vectorField& valuePatch = *valuePatchPtr;

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            valuePatch[i].x() =
                buffer[bufferIndex++];

            // y-dimension
            valuePatch[i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                valuePatch[i].z() =
                    buffer[bufferIndex++];
            }
        }
    }
}

bool preciceAdapter::FF::Velocity::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::Velocity::getDataName() const
{
    return "Velocity";
}
