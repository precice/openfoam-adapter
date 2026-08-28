#include "Stress.H"

using namespace Foam;

preciceAdapter::FSI::Stress::Stress(
    const Foam::fvMesh& mesh,
    const std::string solverType,
    const std::string nameStress)
: ForceBase(mesh, solverType)
{
    // Check if a stress field with the requested name exists.
    // If yes (e.g., solids4Foam), bind Stress_ to that field.
    // If not (e.g., pimpleFoam), create it.
    if (mesh_.foundObject<volVectorField>(nameStress))
    {
        Stress_ =
            &const_cast<volVectorField&>(
                mesh_.lookupObject<volVectorField>(nameStress));
    }
    else
    {
        StressOwning_.reset(new volVectorField(
            IOobject(
                nameStress,
                mesh_.time().timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE),
            mesh,
            dimensionedVector(
                "pdim",
                dimensionSet(1, -1, -2, 0, 0, 0, 0),
                Foam::vector::zero)));

        Stress_ = StressOwning_.get();
    }
}

std::size_t preciceAdapter::FSI::Stress::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    return this->writeToBuffer(buffer, *Stress_, dim);
}

void preciceAdapter::FSI::Stress::read(double* buffer, const unsigned int dim)
{
    this->readFromBuffer(buffer, *Stress_, dim);
}

bool preciceAdapter::FSI::Stress::isLocationTypeSupported(const bool meshConnectivity) const
{
    if (meshConnectivity)
    {
        return false;
    }
    else
    {
        return (this->locationType_ == LocationType::faceCenters);
    }
}

std::string preciceAdapter::FSI::Stress::getDataName() const
{
    return "Stress";
}

Foam::tmp<Foam::vectorField> preciceAdapter::FSI::Stress::getFaceVectors(const unsigned int patchID) const
{
    // face normal vectors
    return mesh_.boundary()[patchID].nf();
}
