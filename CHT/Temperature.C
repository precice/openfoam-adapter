#include "Temperature.H"
#include "primitivePatchInterpolation.H"


using namespace Foam;

preciceAdapter::CHT::Temperature::Temperature(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}

void preciceAdapter::CHT::Temperature::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        forAll(T_->internalField(), i)
        {
            buffer[bufferIndex++] = T_->internalField()[i];
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const scalarField& TPatch(
            T_->boundaryField()[patchID]);

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if (meshConnectivity)
        {
            //Create an Interpolation object at the boundary Field
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            //Interpolate from centers to nodes
            scalarField TPoints(
                patchInterpolator.faceToPointInterpolate(TPatch));

            forAll(TPoints, i)
            {
                // Copy the temperature into the buffer
                buffer[bufferIndex++] =
                    TPoints[i];
            }
        }
        else
        {
            forAll(TPatch, i)
            {
                // Copy the temperature into the buffer
                buffer[bufferIndex++] =
                    TPatch[i];
            }
        }
    }
}

void preciceAdapter::CHT::Temperature::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    if (this->locationType_ == LocationType::volumeCenters)
    {
        forAll(T_->ref(), i)
        {
            T_->ref()[i] = buffer[bufferIndex++];
        }
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(T_->boundaryField()[patchID], i)
        {
            // Set the temperature as the buffer value
            T_->boundaryFieldRef()[patchID][i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::CHT::Temperature::isLocationTypeSupported(const bool meshConnectivity) const
{
    // For cases with mesh connectivity, we support:
    // - face nodes, only for writing
    // - face centers, only for reading
    // However, since we do not distinguish between reading and writing in the code, we
    // always return true and offload the handling to the user.
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::faceNodes); // we do not support meshConnectivity for volumeCenters (yet)
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::volumeCenters);
    }
}

std::string preciceAdapter::CHT::Temperature::getDataName() const
{
    return "Temperature";
}
