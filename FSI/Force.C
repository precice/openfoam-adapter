#include "Force.H"

using namespace Foam;

preciceAdapter::FSI::Force::Force(
    const Foam::fvMesh& mesh,
    const std::string solverType)
: ForceBase(mesh, solverType)
{
    Force_ = new volVectorField(
        IOobject(
            "Force",
            mesh_.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE),
        mesh,
        dimensionedVector(
            "fdim",
            dimensionSet(1, 1, -2, 0, 0, 0, 0),
            Foam::vector::zero));
}

void preciceAdapter::FSI::Force::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    this->writeToBuffer(buffer, *Force_, dim);
}

void preciceAdapter::FSI::Force::read(double* buffer, const unsigned int dim)
{
    this->readFromBuffer(buffer);
}

vectorField preciceAdapter::FSI::Force::getFaceVectors(const unsigned int patchID) const
{
    // Normal vectors multiplied by face area
    return mesh_.boundary()[patchID].Sf();
}

preciceAdapter::FSI::Force::~Force()
{
    delete Force_;
}
