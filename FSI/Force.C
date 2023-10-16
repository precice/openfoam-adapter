#include "Force.H"

using namespace Foam;

preciceAdapter::FSI::Force::Force(
    const Foam::fvMesh& mesh,
    const std::string solverType,
    const std::string nameForce)
: ForceBase(mesh, solverType)
{
    // Check if a force field with the requested name exists.
    // If yes (e.g., solids4Foam), bind Force_ to that field.
    // If not (e.g., pimpleFoam without the Forces function object), create it.
    if (mesh_.foundObject<volVectorField>(nameForce))
    {
        Force_ =
            &const_cast<volVectorField&>(
                mesh_.lookupObject<volVectorField>(nameForce));
    }
    else
    {
        ForceOwning_.reset(new volVectorField(
            IOobject(
                nameForce,
                mesh_.time().timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE),
            mesh,
            dimensionedVector(
                "fdim",
                dimensionSet(1, 1, -2, 0, 0, 0, 0),
                Foam::vector::zero)));

        Force_ = ForceOwning_.get();
    }
}

std::size_t preciceAdapter::FSI::Force::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    return this->writeToBuffer(buffer, *Force_, dim);
}

void preciceAdapter::FSI::Force::read(double* buffer, const unsigned int dim)
{
    // Copy the force field from the buffer to OpenFOAM

    // Here we assume that a force volVectorField exists, which is used by
    // the OpenFOAM solver

    int bufferIndex = 0;
    // Set boundary forces
    for (unsigned int j = 0; j < patchIDs_.size(); j++)
    {
        // Get the ID of the current patch
        const unsigned int patchID = patchIDs_.at(j);

        if (this->locationType_ == LocationType::faceCenters)
        {
            // Make a force field
            vectorField& force = Force_->boundaryFieldRef()[patchID];

            // Copy the forces from the buffer into the force field
            forAll(force, i)
            {
                for (unsigned int d = 0; d < dim; ++d)
                    force[i][d] = buffer[bufferIndex++];
            }
        }
        else if (this->locationType_ == LocationType::faceNodes)
        {
            // Here we could easily interpolate the face values to point values
            // and assign them to some field, but I guess there is no need
            // unless it will be used
            notImplemented("Read forces not implemented for faceNodes!");
        }
    }
}

bool preciceAdapter::FSI::Force::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::faceCenters);
}

std::string preciceAdapter::FSI::Force::getDataName() const
{
    return "Force";
}

Foam::tmp<Foam::vectorField> preciceAdapter::FSI::Force::getFaceVectors(const unsigned int patchID) const
{
    // Normal vectors multiplied by face area
    return mesh_.boundary()[patchID].Sf();
}
