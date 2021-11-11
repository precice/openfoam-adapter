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
    locationsType_ = locationsType;
}

void preciceAdapter::CouplingDataUser::checkDataLocation(const bool meshConnectivity)
{
    if (this->isLocationTypeSupported(meshConnectivity) == false)
    {
        std::string location("none");
        if (locationsType_ == LocationType::faceCenters)
            location = "faceCenters";
        else if (locationsType_ == LocationType::faceNodes)
            location = "faceNodes";

        FatalErrorInFunction
            << "ERROR: the configured location type "
            << location
            << " is not supported for the data "
            << exit(Foam::FatalError);
    }
}

// Dummy implementation which can be overwritten in derived classes if required
void preciceAdapter::CouplingDataUser::initialize()
{
    return;
}
