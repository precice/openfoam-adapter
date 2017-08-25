#include "TemperatureBoundaryCondition.h"

using namespace Foam;

preciceAdapter::TemperatureBoundaryCondition::TemperatureBoundaryCondition( volScalarField * T ) :
	_T( T )
{
    _dataType = scalar;
}

void preciceAdapter::TemperatureBoundaryCondition::read( double * dataBuffer )
{
	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		int patchID = _patchIDs.at( k );
		forAll( _T->boundaryFieldRef()[patchID], i )
		{
			_T->boundaryFieldRef()[patchID][i] = dataBuffer[bufferIndex++];
		}
	}
}
