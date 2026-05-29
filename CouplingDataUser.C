#include "CouplingDataUser.H"

preciceAdapter::CouplingDataUser::CouplingDataUser()
{
}

bool preciceAdapter::CouplingDataUser::hasScalarData()
{
    return dataType_ == scalar;
}

bool preciceAdapter::CouplingDataUser::hasVectorData()
{
    return dataType_ == vector;
}

void preciceAdapter::CouplingDataUser::setDataName(std::string dataName)
{
    dataName_ = std::move(dataName);
}

const std::string& preciceAdapter::CouplingDataUser::dataName()
{
    return dataName_;
}

void preciceAdapter::CouplingDataUser::setFlipNormal(bool flipNormal)
{
    flipNormal_ = flipNormal;
}

void preciceAdapter::CouplingDataUser::applyFlipNormal(precice::span<double> dataBuffer)
{
    if (flipNormal_)
    {
        for (double& val : dataBuffer)
        {
            val *= -1.0;
        }
    }
}

void preciceAdapter::CouplingDataUser::setPatchIDs(std::vector<int> patchIDs)
{
    patchIDs_ = patchIDs;
}

void preciceAdapter::CouplingDataUser::setCellSetNames(std::vector<std::string> cellSetNames)
{
    cellSetNames_ = cellSetNames;
}

void preciceAdapter::CouplingDataUser::setLocationsType(LocationType locationsType)
{
    locationType_ = locationsType;
}

void preciceAdapter::CouplingDataUser::checkDataLocation(const bool meshConnectivity) const
{
    if (this->isLocationTypeSupported(meshConnectivity) == false)
    {
        std::string location("none");
        if (locationType_ == LocationType::faceCenters)
            location = "faceCenters";
        else if (locationType_ == LocationType::faceNodes)
            location = "faceNodes";
        else if (locationType_ == LocationType::volumeCenters)
            location = "volumeCenters";

        if (meshConnectivity)
        {
            adapterInfo("The data \"" + getDataName() + "\""
                            + " does not currently support mesh connectivity (e.g., nearest-projection mapping) for location type "
                            + "\"" + location + "\".",
                        "error");
        }
        else
        {
            adapterInfo("The data \"" + getDataName() + "\" does not support location type \""
                            + location + "\". Please select a different location type.",
                        "error");
        }
    }
}

// Dummy implementation which can be overwritten in derived classes if required
void preciceAdapter::CouplingDataUser::initialize()
{
    return;
}
