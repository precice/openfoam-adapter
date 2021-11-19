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

void preciceAdapter::CouplingDataUser::checkDataLocation(const bool meshConnectivity) const
{
    if (isLocationTypeSupported(meshConnectivity) == false)
    {
        std::string location("none");
        if (locationType_ == LocationType::faceCenters)
            location = "faceCenters";
        else if (locationType_ == LocationType::faceNodes)
            location = "faceNodes";

        FatalErrorInFunction
            << "ERROR: the configured location type "
            << location
            << " is not supported for the data "
            << getDataName()
            << ". Please select a different location type or "
            << "a different data set."
            << exit(Foam::FatalError);
    }
}

// Dummy implementation which can be overwritten in derived classes if required
void preciceAdapter::CouplingDataUser::initialize()
{
    return;
}
