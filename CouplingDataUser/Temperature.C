#include "Temperature.H"

using namespace Foam;

preciceAdapter::User::Temperature::Temperature
(
    volScalarField * T
)
:
T_(T)
{
    dataType_ = scalar;
}

void preciceAdapter::User::Temperature::write(double * buffer)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
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

void preciceAdapter::User::Temperature::read(double * buffer)
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
