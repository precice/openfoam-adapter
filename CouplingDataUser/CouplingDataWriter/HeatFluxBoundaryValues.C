#include "HeatFluxBoundaryValues.h"


preciceAdapter::HeatFluxBoundaryValues::HeatFluxBoundaryValues( Foam::volScalarField & T, double k ) :
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

		Foam::scalarField flux = -_k* Foam::refCast<Foam::fixedValueFvPatchScalarField>( _T.boundaryFieldRef()[patchID] ).snGrad();
		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}
