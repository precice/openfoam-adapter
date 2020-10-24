
#include "HeatTransferCoefficient.H"
#include "primitivePatchInterpolation.H"
#include "volFields.H"

#include "apiCoupledTemperatureFvPatchScalarField.H"

#include <iostream>

using namespace Foam;

//----- preciceAdapter::CHT::HeatTransferCoefficient --------------------------

preciceAdapter::CHT::HeatTransferCoefficient::HeatTransferCoefficient
(
    const Foam::fvMesh& mesh,
    const std::string nameT
)
:
    CouplingDataUser(DT_Scalar),
    T_(mesh.lookupObjectRef<volScalarField>(nameT)),
    mesh_(mesh)
{}

void preciceAdapter::CHT::HeatTransferCoefficient::write(std::vector<double> &buffer, bool meshConnectivity, const unsigned int dim)
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
        std::cout << "Write HTC: ";
        forAll(data, i)
        {
            buffer[bufferIndex++] = data[i];
            std::cout << data[i] << " , ";
        }
        std::cout << std::endl;
    }
}

void preciceAdapter::CHT::HeatTransferCoefficient::read(const std::vector<double> &buffer, const unsigned int dim)
{
    std::size_t bufferIndex = 0;

    // For every boundary patch of the interface
    for (std::size_t j = 0; j < patchIDs_.size(); j++)
    {
        const auto  patchID         (patchIDs_.at(j));
        auto &      boundaryPatch   (refCast<apiCoupledTemperatureFvPatchScalarField> (T_.boundaryFieldRef()[patchID]));
        auto &      patchValue      (boundaryPatch.h_Neighbour());

        // For every cell on the patch
        std::cout << "Read HTC: ";
        forAll(patchValue, i)
        {
            patchValue[i] = buffer[bufferIndex++];
            std::cout << buffer[i] << " , ";
        }
        std::cout << std::endl;
    }
}
