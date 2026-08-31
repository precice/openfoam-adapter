#include "ControlDeflection.H"

#include "pointPatchField.H"
#include "fixedValuePointPatchField.H"

using namespace Foam;

// Rotate a point p about the line through hinge with direction axis by angle
// theta (radians). Uses the Rodrigues rotation formula.
static Foam::vector rotateAboutAxis(
    const Foam::vector& p,
    const Foam::vector& hinge,
    const Foam::vector& axis,
    const scalar theta)
{
    const Foam::vector d = p - hinge;
    const Foam::vector a = normalised(axis);
    const scalar c = Foam::cos(theta);
    const scalar s = Foam::sin(theta);

    return hinge + c*d + s*(a ^ d) + (1.0 - c)*(a & d)*a;
}

preciceAdapter::FSI::ControlDeflection::ControlDeflection(
    const Foam::fvMesh& mesh,
    std::vector<ControlSurfaceConfig> controlSurfaces)
: mesh_(mesh),
  controlSurfaces_(std::move(controlSurfaces))
{
    dataType_ = scalar;

    // Read the name of the pointDisplacement field (if different)
    const dictionary& FSIdict =
        mesh_.lookupObject<IOdictionary>("preciceDict").subOrEmptyDict("FSI");
    namePointDisplacement_ = FSIdict.lookupOrDefault<word>(
        "namePointDisplacement", "pointDisplacement");
}

void preciceAdapter::FSI::ControlDeflection::initialize()
{
    storeReferencePoints();
}

void preciceAdapter::FSI::ControlDeflection::storeReferencePoints()
{
    referencePoints_.clear();

    for (const auto& surface : controlSurfaces_)
    {
        const label patchID = mesh_.boundary().findIndex(surface.patch);

        if (patchID == -1)
        {
            FatalErrorInFunction
                << "Control surface patch '" << surface.patch
                << "' does not exist."
                << exit(FatalError);
        }

        // Initial (reference) positions of the patch points.
        const pointField localPoints =
            mesh_.boundary()[patchID].poly().localPoints();

        std::vector<double> refs;
        refs.reserve(localPoints.size() * 3);
        for (const point& p : localPoints)
        {
            refs.push_back(p.x());
            refs.push_back(p.y());
            refs.push_back(p.z());
        }
        referencePoints_.push_back(std::move(refs));
    }
}

std::size_t preciceAdapter::FSI::ControlDeflection::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    notImplemented("Writing control-surface deflections is not implemented!");
    return 0;
}

void preciceAdapter::FSI::ControlDeflection::read(double* buffer, const unsigned int dim)
{
    // Look up the pointDisplacement field (the dynamic mesh solver consumes it).
    if (!mesh_.foundObject<pointVectorField>(namePointDisplacement_))
        return;

    pointVectorField& pointDisplacement = const_cast<pointVectorField&>(
        mesh_.lookupObject<pointVectorField>(namePointDisplacement_));

    // One deflection angle per control surface, in buffer order.
    for (std::size_t s = 0; s < controlSurfaces_.size(); ++s)
    {
        const ControlSurfaceConfig& surface = controlSurfaces_.at(s);
        const label patchID = mesh_.boundary().findIndex(surface.patch);
        if (patchID == -1)
            continue;

        const Foam::scalar theta = buffer[s];

        const Foam::vector hinge(surface.hinge[0], surface.hinge[1], surface.hinge[2]);
        const Foam::vector axis(surface.axis[0], surface.axis[1], surface.axis[2]);

        // Point boundary field of the control-surface patch.
        vectorField& pField =
            refCast<vectorField>(pointDisplacement.boundaryFieldRef()[patchID]);

        const std::vector<double>& refs = referencePoints_.at(s);

        // Displacement = rotated - reference, so the patch rigidly rotates
        // about the hinge while the rest of the boundary stays fixed.
        forAll(pField, i)
        {
            const Foam::vector p0(refs[3*i + 0], refs[3*i + 1], refs[3*i + 2]);
            const Foam::vector pRot = rotateAboutAxis(p0, hinge, axis, theta);
            pField[i] = pRot - p0;
        }
    }
}

bool preciceAdapter::FSI::ControlDeflection::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::fixedPoints);
}

std::string preciceAdapter::FSI::ControlDeflection::getDataName() const
{
    return "Deflection";
}