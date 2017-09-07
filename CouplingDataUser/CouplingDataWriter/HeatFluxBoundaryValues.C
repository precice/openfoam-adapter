#include "HeatFluxBoundaryValues.h"

using namespace Foam;

preciceAdapter::HeatFluxBoundaryValues::HeatFluxBoundaryValues( volScalarField * T, double k ) :
	T_( T ),
	k_( k )
{
    dataType_ = scalar;
}

void preciceAdapter::HeatFluxBoundaryValues::write( double * dataBuffer )
{

	int bufferIndex = 0;

	for( uint k = 0 ; k < patchIDs_.size() ; k++ )
	{

		int patchID = patchIDs_.at( k );

		scalarField flux = -k_* refCast<fixedValueFvPatchScalarField>( T_->boundaryFieldRef()[patchID] ).snGrad();
		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}
