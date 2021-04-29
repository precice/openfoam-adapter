#include "Stress.H"

using namespace Foam;

preciceAdapter::FSI::Stress::Stress(
    const Foam::fvMesh& mesh,
    const std::string solverType)
: ForceBase(mesh, solverType)
{
    Stress_ = new volVectorField(
        IOobject(
            "Stress",
            mesh_.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE),
        mesh,
        dimensionedVector(
            "pdim",
            dimensionSet(1, -1, -2, 0, 0, 0, 0),
            Foam::vector::zero));
}

void preciceAdapter::FSI::Stress::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    this->writeToBuffer(buffer, *Stress_, dim);
}

void preciceAdapter::FSI::Stress::read(double* buffer, const unsigned int dim)
{
    this->readFromBuffer(buffer);
}

vectorField preciceAdapter::FSI::Stress::getFaceVectors(const unsigned int patchID) const
{
    // face normal vectors
    return mesh_.boundary()[patchID].nf();
}

preciceAdapter::FSI::Stress::~Stress()
{
    delete Stress_;
}
