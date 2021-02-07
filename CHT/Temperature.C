
#include "Temperature.H"

#include "word.H"
#include "primitivePatchInterpolation.H"
#include "volFields.H"

#include "apiCoupledTemperatureFvPatchScalarField.H"

using namespace Foam;

preciceAdapter::CHT::Temperature::Temperature
(
    const Foam::fvMesh& mesh,
    const Foam::word nameT
) :
    CouplingDataUser(),
    T_(mesh.lookupObjectRef<volScalarField>(nameT)),
    mesh_(mesh)
{}

void preciceAdapter::CHT::Temperature::write(double *buffer, bool meshConnectivity, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++ )
    {
        const auto   patchID          (patchIDs_.at(j));
        const auto & boundaryPatch    (T_.boundaryField()[patchID]);

        //If we use the mesh connectivity, we interpolate from the centres to the nodes
        if(meshConnectivity)
        {
            //Create an Interpolation object at the boundary Field
            primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

            //Interpolate from centers to nodes
            const scalarField pointValue (patchInterpolator.faceToPointInterpolate(boundaryPatch));

            forAll(pointValue, i)
            {
                buffer[bufferIndex++] = pointValue[i];
            }
        }
        else
        {
            forAll(boundaryPatch, i)
            {
                buffer[bufferIndex++] = boundaryPatch[i];
            }
        }
    }
}

void preciceAdapter::CHT::Temperature::read(double *buffer, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++)
    {
        const auto  patchID         (patchIDs_.at(j));
        auto &      boundaryPatch   (refCast<apiCoupledTemperatureFvPatchScalarField> (T_.boundaryFieldRef()[patchID]));
        auto &      value           (boundaryPatch.refValue());

        forAll(value, i)
        {
            value[i] = buffer[bufferIndex++];
        }
    }
}
