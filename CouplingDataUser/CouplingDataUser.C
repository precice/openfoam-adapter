#include "CouplingDataUser.h"
#include <iostream>

preciceAdapter::CouplingDataUser::CouplingDataUser()
{
    // set default
    _dataType = scalar;
}

bool preciceAdapter::CouplingDataUser::hasVectorData()
{
	return _dataType == vector;
}

bool preciceAdapter::CouplingDataUser::hasScalarData()
{
	return _dataType == scalar;
}

void preciceAdapter::CouplingDataUser::setSize( int size )
{
	_bufferSize = size;
}

void preciceAdapter::CouplingDataUser::setPatchIDs( std::vector<int> patchIDs )
{
	_patchIDs = patchIDs;
}

void preciceAdapter::CouplingDataUser::setDataID( int dataID )
{
	_dataID = dataID;
}

int preciceAdapter::CouplingDataUser::getDataID()
{
	return _dataID;
}
