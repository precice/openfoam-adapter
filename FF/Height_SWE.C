#include "Height_SWE.H"

using namespace Foam;

preciceAdapter::FF::height_SWE::height_SWE
(
    const Foam::fvMesh& mesh,
    const std::string nameA_,
    const std::string namePrgh_
    // const std::string nameP_
)
:
alpha_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(nameA_)
    )
),
p_rgh_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(namePrgh_)
    )
)
{
    dataType_ = scalar;
    mesh_ = &mesh;
}

void preciceAdapter::FF::height_SWE::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{//TODO
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
            // Copy the HU into the buffer
            buffer[bufferIndex++]
            =
            alpha_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::height_SWE::read(double * buffer, const unsigned int dim)
{
    // int count = 0;
    // for (size_t i = 0; i < 20; i++) {
    //     for (size_t j = 0; j < 20; j++) {
    //         std::cout << buffer[count++] << " ";
    //     }
    //     std::cout << '\n';
    // }

    int bufferIndex = 0;

    //TODO get the densities and gravity from transportPropertiesDict
     double rho_water = 1000.0;
     double rho_air = 1.0;
     double g = 9.81;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        //get face centres of boundary mesh, and then use their y-coordinate
        const vectorField faceCenters =
            mesh_->boundaryMesh()[patchID].faceCentres();

        //NOTE alpha and p_rgh will be set in the same loop, but the loop
        // is only iterating alpha, should be the same with p_rgh
        forAll(alpha_->boundaryFieldRef()[patchID], i)
        {
        //*****Set alpha from SWE height*****
        // std::cout << "faceCenter: " << faceCenters[i].y() << " Cell#: " << i << " buffer: " << buffer[bufferIndex] << '\n';

            //Mintgen's algorithm for alpha
            double alphaLocal = 0.0;

            if (faceCenters[i].y() >= buffer[bufferIndex]){
                //do nothing, alphaLocal is already 0.0

            } else if (faceCenters[i].y() <= buffer[bufferIndex] ){
                alphaLocal = 1.0;

            } else {
                alphaLocal =
                0.5 + (buffer[bufferIndex] - faceCenters[i].y()) / this->getBoundaryCellSize();

            }

            // Set alpha, TODO cast?
            alpha_->boundaryFieldRef()[patchID][i] = alphaLocal;

        //*****Set p_rgh from SWE height*****
            // double p = p_->boundaryFieldRef()[patchID][i];
            double rho_mixed = rho_water * alphaLocal + rho_air * (1 - alphaLocal);

            // Set p_rgh according to page 85, eq 5.16 mintgen, TODO cast?
            p_rgh_->boundaryFieldRef()[patchID][i] =
                // p - rho_mixed * g * buffer[bufferIndex];
                rho_mixed * g * buffer[bufferIndex];


            bufferIndex++;
        }

    }
}

double preciceAdapter::FF::height_SWE::getBoundaryCellSize()
{
    //Ugly workaround for getting cell size.
    //It works only for uniform grids and cellsize bigger than 0.000001
    //TODO find a way to get cellsize from OF methods

    const surfaceVectorField& deltas = mesh_->delta();
    double delta{-1};
    forAll(deltas, I){
        if(deltas[I].x() > 0.000001){
            delta = static_cast<double>(deltas[I].x());
            break;
        }
    }

    return delta;
}
