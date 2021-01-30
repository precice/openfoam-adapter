
#include "HeatTransferCoefficient.H"

#include "word.H"
#include "primitivePatchInterpolation.H"
#include "volFields.H"

#include "apiCoupledTemperatureFvPatchScalarField.H"

using namespace Foam;

//----- preciceAdapter::CHT::HeatTransferCoefficient --------------------------

preciceAdapter::CHT::HeatTransferCoefficient::HeatTransferCoefficient
(
    const Foam::fvMesh& mesh,
    const Foam::word nameT
)
:
    CouplingDataUser(),
    T_(mesh.lookupObjectRef<volScalarField>(nameT)),
    mesh_(mesh)
{}

void preciceAdapter::CHT::HeatTransferCoefficient::write(double *buffer, bool meshConnectivity, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++ )
    {
        const auto      patchID         (patchIDs_.at(j));
        const auto &    boundaryPatch   (refCast<const apiCoupledTemperatureFvPatchScalarField> (T_.boundaryField()[patchID]));
        auto            value           (boundaryPatch.getHeatTransferCoeff());

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if(meshConnectivity)
        {
            //Setup Interpolation object
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            //Interpolate
            value = patchInterpolator.faceToPointInterpolate(value);
        }

        //
        const scalarField & data (value.cref());
        forAll(data, i)
        {
            buffer[bufferIndex++] = data[i];
        }
    }
}

void preciceAdapter::CHT::HeatTransferCoefficient::read(double *buffer, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++)
    {
        const auto  patchID         (patchIDs_.at(j));
        auto &      boundaryPatch   (refCast<apiCoupledTemperatureFvPatchScalarField> (T_.boundaryFieldRef()[patchID]));
        auto &      patchValue      (boundaryPatch.h_Neighbour());

        // For every cell on the patch
        forAll(patchValue, i)
        {
            patchValue[i] = buffer[bufferIndex++];
        }
    }
}
