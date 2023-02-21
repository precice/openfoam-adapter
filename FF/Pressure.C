#include "Pressure.H"
#include "coupledPressureFvPatchField.H"

using namespace Foam;

preciceAdapter::FF::Pressure::Pressure(
    const Foam::fvMesh& mesh,
    const std::string nameP)
: p_(
    const_cast<volScalarField*>(
        &mesh.lookupObject<volScalarField>(nameP)))
{
    dataType_ = scalar;
}

void preciceAdapter::FF::Pressure::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // For every cell of the patch
        forAll(p_->boundaryFieldRef()[patchID], i)
        {
            // Copy the pressure into the buffer
            buffer[bufferIndex++] =
                p_->boundaryFieldRef()[patchID][i];
        }
    }
}

void preciceAdapter::FF::Pressure::read(double* buffer, const unsigned int dim)
{
    int bufferIndex = 0;
    if (firstRead) {
        firstRead = false;
        return;
    }

    // For every boundary patch of the interface
    for (uint j = 0; j < patchIDs_.size(); j++)
    {
        int patchID = patchIDs_.at(j);

        // Get the pressure value boundary patch
        scalarField* valuePatchPtr = &p_->boundaryFieldRef()[patchID];
        if (isA<coupledPressureFvPatchField>(p_->boundaryFieldRef()[patchID]))
        {
            valuePatchPtr = &refCast<coupledPressureFvPatchField>(
                                 p_->boundaryFieldRef()[patchID])
                                 .refValue();
        }
        scalarField& valuePatch = *valuePatchPtr;

        // For every cell of the patch
        forAll(p_->boundaryFieldRef()[patchID], i)
        {
            // Set the pressure as the buffer value
            valuePatch[i] =
                buffer[bufferIndex++];
        }
    }
}

bool preciceAdapter::FF::Pressure::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FF::Pressure::getDataName() const
{
    return "Pressure";
}
