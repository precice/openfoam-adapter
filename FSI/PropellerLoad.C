#include "PropellerLoad.H"

#include "fvModels.H"
#include "propellerDisk.H"
#include "vector.H"

using namespace Foam;

preciceAdapter::FSI::PropellerLoad::PropellerLoad(
    const Foam::fvMesh& mesh,
    std::vector<std::string> propellerNames,
    bool isMoment)
: mesh_(mesh),
  propellerNames_(std::move(propellerNames)),
  isMoment_(isMoment)
{
    dataType_ = vector;
}

std::size_t preciceAdapter::FSI::PropellerLoad::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    // Get the fvModels object (contains all the fvModels of the case)
    const fvModels& models = mesh_.lookupObject<fvModels>("fvModels");

    std::size_t bufferIndex = 0;

    // Write one value (thrust or torque) per propeller hub, in the same
    // order as the propeller fvModels were specified.
    for (const std::string& name : propellerNames_)
    {
        const fv::propellerDisk& prop = dynamicCast<const fv::propellerDisk>(models[name]);

        // Sign convention (aircraft side):
        // The propellerDisk source adds momentum to the FLUID in the +normal
        // (thrust) direction; the reaction on the airframe is therefore
        // in the -normal direction. We pass the aircraft-side thrust as
        // -prop.force() and the reaction torque as -prop.moment().
        const Foam::vector value = isMoment_ ? -prop.moment() : -prop.force();

        for (unsigned int d = 0; d < dim; ++d)
            buffer[bufferIndex++] = value[d];
    }

    return bufferIndex;
}

void preciceAdapter::FSI::PropellerLoad::read(double* buffer, const unsigned int dim)
{
    notImplemented("Reading propeller loads is not implemented!");
}

bool preciceAdapter::FSI::PropellerLoad::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::fixedPoints);
}

std::string preciceAdapter::FSI::PropellerLoad::getDataName() const
{
    return isMoment_ ? "PropTorque" : "Thrust";
}