#include "TemperatureAIT.H"

using namespace Foam;

preciceAdapter::AITS::Temperature::Temperature
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

void preciceAdapter::AITS::Temperature::write(double * buffer, bool meshConnectivity)      //Bernhard Version 2
{
    int bufferIndex = 0;

    const volScalarField & Temprtr_ = *T_;

    // For every cell of the mesh
    forAll(Temprtr_, i)
    {
        // Copy the temperature into the buffer
        buffer[bufferIndex++]
        =
        Temprtr_[i];
    }

    for (uint j = 0; j < patchIDs_.size(); j++ )
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(T_->boundaryFieldRef()[patchID], i)
        {
            // Copy the temperature into the buffer
            buffer[bufferIndex++]
            =
            T_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::AITS::Temperature::read(double * buffer)     // Bernhard Version 2
{
    int bufferIndex = 0;

    volScalarField &Temprtr_ = *T_;

    // For every cell of the mesh
    forAll(Temprtr_, i)
    {
        // Set the temperature as the buffer value
        Temprtr_[i]
        =
        buffer[bufferIndex++];
    }
    *T_ = Temprtr_;


    for (uint j = 0; j < patchIDs_.size(); j++ )
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
