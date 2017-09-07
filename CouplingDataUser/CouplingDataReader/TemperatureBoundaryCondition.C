#include "TemperatureBoundaryCondition.H"

using namespace Foam;

preciceAdapter::
TemperatureBoundaryCondition::
TemperatureBoundaryCondition(
    volScalarField * T
)
:
T_(T)
{
    dataType_ = scalar;
}

void preciceAdapter::
TemperatureBoundaryCondition::
read(double * dataBuffer)
{
    int bufferIndex = 0;

    for(uint k = 0; k < patchIDs_.size(); k++)
    {
        int patchID = patchIDs_.at(k);

        forAll(T_->boundaryFieldRef()[patchID], i)
        {
            T_->boundaryFieldRef()[patchID][i] = dataBuffer[bufferIndex++];
        }
    }
}
