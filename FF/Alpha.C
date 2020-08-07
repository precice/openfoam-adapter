#include "Alpha.H"

using namespace Foam;

preciceAdapter::FF::Alpha::Alpha
(
    const Foam::fvMesh& mesh,
    const std::string nameA
)
:
alpha_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameA)
    )
)
{
    dataType_ = scalar;
    mesh_ = &mesh;
}

void preciceAdapter::FF::Alpha::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
            // Copy the alpha into the buffer
            buffer[bufferIndex++]
            =
            alpha_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Alpha::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        const vectorField faceCenters =
            mesh_->boundaryMesh()[patchID].faceCentres();

        // For every cell of the patch
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
            //Mintgens algorithm for alpha
            if (buffer[bufferIndex] <= faceCenters[i].y()){
                buffer[bufferIndex] = 0.0;

            } else if (buffer[bufferIndex] >= faceCenters[i].y()){
                buffer[bufferIndex] = 1.0;

            } else {
                buffer[bufferIndex] = 0.5 + (buffer[bufferIndex] - faceCenters[i].y()) / this->getBoundaryCellSize();

            }

            // Set the alpha as the buffer value
            alpha_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++];
        }
    }
}

double preciceAdapter::FF::Alpha::getBoundaryCellSize(){
    //Ugly workaround for getting cell size.
    //It works only for uniform grids and cellsize bigger than 0.000001

    const surfaceVectorField& deltas = mesh_->delta();
    double delta;
    forAll(deltas, I){
        if(deltas[I].x() > 0.000001){
            return static_cast<double>(deltas[I].x());
        }
    }
}
