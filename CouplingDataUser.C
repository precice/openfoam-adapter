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

void preciceAdapter::CouplingDataUser::setDataID(int dataID)
{
    dataID_ = dataID;

    return;
}

int preciceAdapter::CouplingDataUser::dataID()
{
    return dataID_;
}

void preciceAdapter::CouplingDataUser::setPatchIDs(std::vector<int> patchIDs)
{
    patchIDs_ = patchIDs;
}

void preciceAdapter::CouplingDataUser::setLocationsType(LocationType locationsType)
{
    locationType_ = locationsType;
}

void preciceAdapter::CouplingDataUser::writeGradients(std::vector<double>& gradientBuffer, const unsigned int dim)
{
    adapterInfo("Data \"" + getDataName() + "\" does not support writing gradients. Please select a different "
                                      "data or a different mapping configuration, which does not require "
                                      "additional gradient information.",
                "error");
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

        adapterInfo("\"locations = " + location + "\" is not supported for the data \""
                        + getDataName() + "\". Please select a different "
                                          "location type, a different data set or provide "
                                          "additional connectivity information.",
                    "error");
    }
}

// Dummy implementation which can be overwritten in derived classes if required
void preciceAdapter::CouplingDataUser::initialize()
{
    return;
}
