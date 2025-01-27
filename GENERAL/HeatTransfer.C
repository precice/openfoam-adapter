#include "HeatTransfer.H"
#include "primitivePatchInterpolation.H"
#include "fvCFD.H"

#include "mixedFvPatchFields.H"
#include "fixedValueFvPatchFields.H"

using namespace Foam;

//----- preciceAdapter::MODULE::HeatTransfer -----------------------------------------

preciceAdapter::MODULE::HeatTransfer::HeatTransfer(
    const Foam::fvMesh& mesh,
    const std::string nameT)
: T_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameT))),
  mesh_(mesh)
{
    dataType_ = scalar;
}

std::size_t preciceAdapter::MODULE::HeatTransfer::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const scalarField gradientPatch(
            (T_->boundaryField()[patchID])
                .snGrad());

        // If we use the mesh connectivity, we interpolate from the centres to the nodes
        if (meshConnectivity)
        {
            //Setup Interpolation object
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            scalarField gradientPoints;

            //Interpolate
            gradientPoints = patchInterpolator.faceToPointInterpolate(gradientPatch);

            // // For every cell of the patch
            // forAll(gradientPoints, i)
            // {
            //     // Copy the heat flux into the buffer
            //     // Q = - k * gradient(T)
            //     //TODO: Interpolate kappa in case of a turbulent calculation
            //     buffer[bufferIndex++] =
            //         -getKappaEffAt(i) * gradientPoints[i];
            // }
        }
        else
        {
            // // For every cell of the patch
            // forAll(gradientPatch, i)
            // {
            //     // Copy the heat flux into the buffer
            //     // Q = - k * gradient(T)
            //     //TODO: Interpolate kappa in case of a turbulent calculation
            //     buffer[bufferIndex++] =
            //         -getKappaEffAt(i) * gradientPatch[i];
            // }
        }
    }
    return bufferIndex;
}

void preciceAdapter::MODULE::HeatTransfer::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the temperature gradient boundary patch
        // scalarField& gradientPatch(
        //     refCast<fixedGradientFvPatchScalarField>(
        //         T_->boundaryFieldRef()[patchID])
        //         .gradient());

        scalarField& gradientPatch(
            refCast<fixedGradientFvPatchScalarField>(
                T_->boundaryFieldRef()[patchID])
                .gradient());

        // // For every cell of the patch
        // forAll(gradientPatch, i)
        // {
        //     // Compute and assign the gradient from the buffer.
        //     // The sign of the heat flux needs to be inversed,
        //     // as the buffer contains the flux that enters the boundary:
        //     // gradient(T) = -Q / -k
        //     gradientPatch[i] =
        //         buffer[bufferIndex++] / getKappaEffAt(i);
        // }
    }
}

bool preciceAdapter::MODULE::HeatTransfer::isLocationTypeSupported(const bool meshConnectivity) const
{
    // For cases with mesh connectivity, we support:
    // - face nodes, only for writing
    // - face centers, only for reading
    // However, since we do not distinguish between reading and writing in the code, we
    // always return true and offload the handling to the user.
    if (meshConnectivity)
    {
        return (this->locationType_ == LocationType::faceCenters || this->locationType_ == LocationType::faceNodes); // we currently do not support meshConnectivity for volumeCenters
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters);
    }
}

std::string preciceAdapter::MODULE::HeatTransfer::getDataName() const
{
    return "Temperature";
}
