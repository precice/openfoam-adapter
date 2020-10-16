#include "CouplingDataUser.H"

preciceAdapter::CouplingDataUser::CouplingDataUser(DataType type)
:
    dataType_(type),
    patchIDs_(),
    dataID_(0)
{}

bool preciceAdapter::CouplingDataUser::hasScalarData()
{
    return dataType_ == DT_Scalar;
}

bool preciceAdapter::CouplingDataUser::hasVectorData()
{
    return dataType_ == DT_Vector;
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
