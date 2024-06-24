#include "PressureGradient.H"   

using namespace Foam;

preciceAdapter::FF::PressureGradient::PressureGradient(
    const Foam::fvMesh& mesh,
    const std::string namePG)
{
    if (mesh.foundObject<volVectorField>(namePG))
    {
        adapterInfo("Using existing velocity object " + namePG, "debug");
        gradp_ = const_cast<volVectorField*>(
            &mesh.lookupObject<volVectorField>(namePG));
    }
    else
    {
        adapterInfo("Creating a new velocity object " + namePG, "debug");
        gradp_ = new volVectorField(
            IOobject(
                namePG,
                mesh.time().timeName(),
                mesh,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE),
            mesh);
    }
    dataType_ = vector;
    //dataType_ = scalar;
}

std::size_t preciceAdapter::FF::PressureGradient::write(double* buffer, bool meshConnectivity, const unsigned int dim)
{
    int bufferIndex = 0;

     if (this->locationType_ == LocationType::volumeCenters)
        {
            for (const auto& cellSetName : cellSetNames_)
            {
                cellSet overlapRegion(gradp_->mesh(), cellSetName);
                const labelList& cells = overlapRegion.toc();

                for (const auto& currentCell : cells)
                {
                    // x-dimension
                    buffer[bufferIndex++] = gradp_->internalField()[currentCell].x();

                    // y-dimension
                    buffer[bufferIndex++] = gradp_->internalField()[currentCell].y();

                    if (dim == 3)
                    {
                        // z-dimension
                        buffer[bufferIndex++] = gradp_->internalField()[currentCell].z();
                    }
                }
            }
        }
    return bufferIndex;
}

void preciceAdapter::FF::PressureGradient::read(double* buffer, const unsigned int dim)
{
}

bool preciceAdapter::FF::PressureGradient::isLocationTypeSupported(const bool meshConnectivity) const
{
    return (this->locationType_ == LocationType::volumeCenters);
}

std::string preciceAdapter::FF::PressureGradient::getDataName() const
{
    return "PressureGradient";
}
