#include "SurfaceHinge.H"

using namespace Foam;

preciceAdapter::FSI::SurfaceHinge::SurfaceHinge(
    const Foam::fvMesh& mesh,
    const std::string solverType,
    std::vector<ControlSurfaceConfig> controlSurfaces,
    bool isMoment)
: ForceBase(mesh, solverType),
  controlSurfaces_(std::move(controlSurfaces)),
  isMoment_(isMoment)
{
    dataType_ = vector;
}

std::size_t preciceAdapter::FSI::SurfaceHinge::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    // Compute the deviatoric stress tensor boundary field and the density
    // boundary field once (they are patch-independent).
    tmp<surfaceVectorField> tdevTau(devTau());
    const surfaceVectorField::Boundary& devRhoReffb(tdevTau().boundaryField());

    tmp<volScalarField> trho(rho());
    const volScalarField::Boundary& rhob = trho().boundaryField();

    const auto& pb = mesh_.lookupObject<volScalarField>("p").boundaryField();

    std::size_t bufferIndex = 0;

    // One value per control surface.
    for (const auto& surface : controlSurfaces_)
    {
        const label patchID = mesh_.boundary().findIndex(surface.patch);
        if (patchID == -1)
            continue;

        tmp<vectorField> tsurface = getFaceVectors(patchID);
        const auto& surfaceV = tsurface();

        const Foam::vector hinge(
            surface.hinge[0], surface.hinge[1], surface.hinge[2]);

        // Total force of the fluid on the control surface.
        Foam::vector totalF(Zero);
        // Moment of that force about the hinge point.
        Foam::vector totalM(Zero);

        forAll(surfaceV, i)
        {
            Foam::vector f(Zero);
            if (solverType_.compare("incompressible") == 0)
            {
                f = surfaceV[i] * pb[patchID][i] * rhob[patchID][i];
            }
            else if (solverType_.compare("compressible") == 0)
            {
                f = surfaceV[i] * pb[patchID][i];
            }

            f += mesh_.magSf().boundaryField()[patchID][i] * devRhoReffb[patchID][i];

            totalF += f;

            const Foam::vector centre = mesh_.boundary()[patchID].poly().faceCentres()[i];
            totalM += (centre - hinge) ^ f;
        }

        const Foam::vector value = isMoment_ ? totalM : totalF;
        for (unsigned int d = 0; d < dim; ++d)
            buffer[bufferIndex++] = value[d];
    }

    return bufferIndex;
}

void preciceAdapter::FSI::SurfaceHinge::read(double* buffer, const unsigned int dim)
{
    notImplemented("Reading control-surface hinge loads is not implemented!");
}

bool preciceAdapter::FSI::SurfaceHinge::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::fixedPoints);
}

std::string preciceAdapter::FSI::SurfaceHinge::getDataName() const
{
    return isMoment_ ? "HingeMoment" : "HingeForce";
}

Foam::tmp<Foam::vectorField> preciceAdapter::FSI::SurfaceHinge::getFaceVectors(const unsigned int patchID) const
{
    // Normal vectors multiplied by face area
    return mesh_.boundary()[patchID].Sf();
}