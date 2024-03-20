#include "VelocityGradient.H"
#include "coupledVelocityFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::VelocityGradient::VelocityGradient(
    const Foam::fvMesh& mesh,
    const std::string nameU)
: U_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>(nameU)))
{
    dataType_ = vector;
}

std::size_t preciceAdapter::FF::VelocityGradient::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity gradient boundary patch
        vectorField gradientPatch((U_->boundaryFieldRef()[patchID])
                                      .snGrad());

        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Copy the velocity into the buffer
            // x-dimension
            buffer[bufferIndex++] =
                -gradientPatch[i].x();

            // y-dimension
            buffer[bufferIndex++] =
                -gradientPatch[i].y();

            if (dim == 3)
            {
                // z-dimension
                buffer[bufferIndex++] =
                    -gradientPatch[i].z();
            }
        }
    }
    return bufferIndex;
}

void preciceAdapter::FF::VelocityGradient::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the velocity gradient boundary patch
        vectorField* gradientPatchPtr;
        if (isA<coupledVelocityFvPatchField>(U_->boundaryFieldRef()[patchID]))
        {
            gradientPatchPtr = &refCast<coupledVelocityFvPatchField>(
                                    U_->boundaryFieldRef()[patchID])
                                    .refGrad();
        }
        else
        {
            gradientPatchPtr = &refCast<fixedGradientFvPatchVectorField>(
                                    U_->boundaryFieldRef()[patchID])
                                    .gradient();
        }
        vectorField& gradientPatch = *gradientPatchPtr;


        // For every cell of the patch
        forAll(gradientPatch, i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            gradientPatch[i].x() =
                buffer[bufferIndex++];

            // y-dimension
            gradientPatch[i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
                // z-dimension
                gradientPatch[i].z() =
                    buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::VelocityGradient::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::VelocityGradient::getDataName() const
{
    return "VelocityGradient";
}
