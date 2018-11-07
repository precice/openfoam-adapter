#include "Temperature.H"
#include "primitivePatchInterpolation.H"


using namespace Foam;

preciceAdapter::CHT::Temperature::Temperature
(
        const Foam::fvMesh& mesh,
        const std::string nameT
        )
    :
      T_(
          const_cast<volScalarField*>
          (
              &mesh.lookupObject<volScalarField>(nameT)
              )
          ),

      mesh_(mesh)

{
    dataType_ = scalar;

}

void preciceAdapter::CHT::Temperature::write(double * buffer)
{

    //If we have a nearest projection mapping, we interpolate from the centres to the nodes
    //TODO: Add the locationsType in the class
    //      in order to make the interpolation only for the required locationstype (faceTriangles)
    //      Currently faceTriangles assumed

    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        //Create an Interpolation object at the boundary Field
        primitivePatchInterpolation patchInterpolator(mesh_.boundaryMesh()[patchID]);

        const scalarField& Tpatch=T_->boundaryFieldRef()[patchID];

        Field<double>  Tpoint;

        //Interpolate from Centers to Nodes
        Tpoint= patchInterpolator.faceToPointInterpolate(Tpatch);


        forAll(Tpoint, i)
        {

            // Set the temperature as the buffer value
            // Copy the temperature into the buffer
            buffer[bufferIndex++]
                    =
                    Tpoint[i];

        }
    }
}

void preciceAdapter::CHT::Temperature::read(double * buffer)
{
    int bufferIndex = 0;


    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);


        // For every cell of the patch
        forAll(T_->boundaryFieldRef()[patchID], i)
        {
            // Set the temperature as the buffer value
            T_->boundaryFieldRef()[patchID][i]
                    =
                    buffer[bufferIndex++];

        }
    }
}
