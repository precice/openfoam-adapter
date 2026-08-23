#include "Velocity.H"

using namespace Foam;

preciceAdapter::FSI::Velocity::Velocity(const Foam::fvMesh& mesh)
: U_(
    const_cast<volVectorField*>(
        &mesh.lookupObject<volVectorField>("U"))),
  mesh_(mesh)
{
    dataType_ = vector;
}

std::size_t preciceAdapter::FSI::Velocity::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    // The airframe velocity is only read from preCICE (it is written by the
    // kinematics participant). Nothing to write here.
    return 0;
}

void preciceAdapter::FSI::Velocity::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;
    for (const label patchID : patchIDs_)
    {
        if (this->locationType_ == LocationType::faceCenters)
        {
            // Copy the rigid-body surface velocity from the buffer into the
            // U boundary field. The solver uses these values as the
            // (moving wall) boundary condition on the airframe patch.
            vectorField& UfluidPatch = U_->boundaryFieldRef()[patchID];
            forAll(UfluidPatch, i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    UfluidPatch[i][d] = buffer[bufferIndex++];
            }
        }
        else
        {
            FatalErrorInFunction
                << "Reading the airframe velocity is only supported for "
                << "faceCenters."
                << exit(FatalError);
        }
    }
}

bool preciceAdapter::FSI::Velocity::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FSI::Velocity::getDataName() const
{
    return "Velocity";
}