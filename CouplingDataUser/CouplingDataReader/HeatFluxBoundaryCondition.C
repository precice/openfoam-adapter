#include "HeatFluxBoundaryCondition.h"

using namespace Foam;

preciceAdapter::HeatFluxBoundaryCondition::HeatFluxBoundaryCondition( volScalarField * T, double k ) :
	_T( T ),
	_k( k )
{
	_dataType = scalar;
}

void preciceAdapter::HeatFluxBoundaryCondition::read( double * dataBuffer )
{
	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		fixedGradientFvPatchScalarField & gradientPatch =
			refCast<fixedGradientFvPatchScalarField>( _T->boundaryFieldRef()[patchID] );

		forAll( gradientPatch, i )
		{
			gradientPatch.gradient()[i] = dataBuffer[bufferIndex++] / _k;
		}

	}
}
