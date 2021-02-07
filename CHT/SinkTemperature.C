
#include "SinkTemperature.H"

#include "word.H"
#include "primitivePatchInterpolation.H"
#include "volFields.H"

#include "OpenFOAMSettings.H"
#include "apiCoupledTemperatureFvPatchScalarField.H"

using namespace Foam;

preciceAdapter::CHT::SinkTemperature::SinkTemperature
(
    const Foam::fvMesh& mesh,
    const Foam::word nameT
) :
    CouplingDataUser(),
    T_(mesh.lookupObjectRef<volScalarField>(nameT)),
    mesh_(mesh)
{}

void preciceAdapter::CHT::SinkTemperature::write(double *buffer, bool meshConnectivity, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++ )
    {
        const auto      patchID         (patchIDs_.at(j));
        const auto &    boundaryPatch   (refCast<const apiCoupledTemperatureFvPatchScalarField> (T_.boundaryField()[patchID]));
        auto            value           (boundaryPatch.T_Cell());

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if(meshConnectivity)
        {
            //Create an Interpolation object at the boundary Field
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            //Interpolate from centers to nodes
            value = patchInterpolator.faceToPointInterpolate(value);
        }
        
        //
#if (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotCOM) && (OpenFOAM_VERSION_MAJOR >= 1806)
        const scalarField & data (value.cref());
#else
        scalarField & data (value.ref());
#endif
        forAll(data, i)
        {
            buffer[bufferIndex++] = data[i];
        }
    }
}

void preciceAdapter::CHT::SinkTemperature::read(double *buffer, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++)
    {
        const auto  patchID         (patchIDs_.at(j));
        auto&       boundaryPatch   (refCast<apiCoupledTemperatureFvPatchScalarField> (T_.boundaryFieldRef()[patchID]));
        auto&       patchValue      (boundaryPatch.T_Neighbour());

        // For every cell of the patch
        forAll(patchValue, i)
        {
            patchValue[i] = buffer[bufferIndex++];
        }
    }
}
