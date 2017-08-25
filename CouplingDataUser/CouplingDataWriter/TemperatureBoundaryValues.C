#include "TemperatureBoundaryValues.h"

using namespace Foam;

preciceAdapter::TemperatureBoundaryValues::TemperatureBoundaryValues( volScalarField * T ) :
	_T( T )
{
    _dataType = scalar;
}

void preciceAdapter::TemperatureBoundaryValues::write( double * dataBuffer )
{
	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		int patchID = _patchIDs.at( k );
		forAll( _T->boundaryFieldRef()[patchID], i )
		{
			dataBuffer[bufferIndex++] = _T->boundaryFieldRef()[patchID][i];
		}
	}
}
