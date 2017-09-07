#include "TemperatureBoundaryValues.H"

using namespace Foam;

preciceAdapter::
TemperatureBoundaryValues::
TemperatureBoundaryValues(
    volScalarField * T
)
:
T_(T)
{
    dataType_ = scalar;
}

void preciceAdapter::TemperatureBoundaryValues::write(double * dataBuffer)
{
    int bufferIndex = 0;

    for(uint k = 0; k < patchIDs_.size(); k++)
    {
        int patchID = patchIDs_.at(k);

        forAll(T_->boundaryFieldRef()[patchID], i)
        {
            dataBuffer[bufferIndex++] = T_->boundaryFieldRef()[patchID][i];
        }
    }
}
