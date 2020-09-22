#include "Gh.H"

using namespace Foam;

preciceAdapter::FF::Gh::Gh
(
    const Foam::fvMesh& mesh,
    const std::string namePrgh
)
:
p_rgh_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>(namePrgh)
    )
),
alpha_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>("alpha.water")
    )
),
p_(
    const_cast<volScalarField*>
    (
        &mesh.lookupObject<volScalarField>("p")
    )
)
{
    dataType_ = scalar;
}

void preciceAdapter::FF::Gh::write(double * buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_rgh_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++]
            =
            p_rgh_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Gh::read(double * buffer, const unsigned int dim)
{
    int bufferIndex = 0;
    double rho_water = 1000.0; // TODO get from OF
    double rho_air = 1.0; // TODO get from OF

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_rgh_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            // std::cout << buffer[bufferIndex] << '\n';

            // p_->boundaryFieldRef()[patchID][i]
            p_rgh_->boundaryFieldRef()[patchID][i]
            =
            buffer[bufferIndex++] * (rho_water * std::abs(alpha_->boundaryFieldRef()[patchID][i]) + rho_air * (1 - std::abs(alpha_->boundaryFieldRef()[patchID][i])));
        }
    }
}
