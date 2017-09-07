#include "HeatFluxBoundaryCondition.H"

using namespace Foam;

preciceAdapter::
HeatFluxBoundaryCondition::
HeatFluxBoundaryCondition
(
    volScalarField * T,
    double k
)
:
T_(T),
k_(k)
{
    dataType_ = scalar;
}

void preciceAdapter::
HeatFluxBoundaryCondition::
read(double * dataBuffer)
{
    int bufferIndex = 0;

    for(uint k = 0; k < patchIDs_.size(); k++)
    {
        int patchID = patchIDs_.at(k);

        fixedGradientFvPatchScalarField & gradientPatch =
        refCast<fixedGradientFvPatchScalarField>(T_->boundaryFieldRef()[patchID]);

        forAll(gradientPatch, i)
        {
            gradientPatch.gradient()[i] = dataBuffer[bufferIndex++] / k_;
        }
    }
}
