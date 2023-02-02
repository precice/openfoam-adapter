#include "Velocity.H"

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

        scalarField phip = phi_->boundaryFieldRef()[patchID];
        vectorField n = U_->boundaryField()[patchID].patch().nf();
        scalarField magS = U_->boundaryField()[patchID].patch().magSf();
        vectorField u_corrected = U_->boundaryField()[patchID] - 
            n*(n & U_->boundaryField()[patchID]) + n*phip/magS;

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

        // For every cell of the patch
        forAll(U_->boundaryFieldRef()[patchID], i)
        {
            // Set the velocity as the buffer value
            // x-dimension
            U_->boundaryFieldRef()[patchID][i].x() =
                buffer[bufferIndex++];

            // y-dimension
            U_->boundaryFieldRef()[patchID][i].y() =
                buffer[bufferIndex++];

            if (dim == 3)
            {
                // z-dimension
                U_->boundaryFieldRef()[patchID][i].z() =
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
