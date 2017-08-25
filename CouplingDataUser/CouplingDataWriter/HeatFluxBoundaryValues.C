#include "HeatFluxBoundaryValues.h"

using namespace Foam;

preciceAdapter::HeatFluxBoundaryValues::HeatFluxBoundaryValues( volScalarField * T, double k ) :
	_T( T ),
	_k( k )
{
    _dataType = scalar;
}

void preciceAdapter::HeatFluxBoundaryValues::write( double * dataBuffer )
{

	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		scalarField flux = -_k* refCast<fixedValueFvPatchScalarField>( _T->boundaryFieldRef()[patchID] ).snGrad();
		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}
