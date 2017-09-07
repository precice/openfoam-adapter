#include "BuoyantPimpleHeatFluxBoundaryValues.h"

using namespace Foam;

preciceAdapter::BuoyantPimpleHeatFluxBoundaryValues::BuoyantPimpleHeatFluxBoundaryValues( volScalarField * T, basicThermo * thermo, compressible::turbulenceModel * turbulence ) :
	T_( T ),
	thermo_( thermo ),
	turbulence_( turbulence )
{
    dataType_ = scalar;
}

void preciceAdapter::BuoyantPimpleHeatFluxBoundaryValues::write( double * dataBuffer )
{

	int bufferIndex = 0;

	for( uint k = 0 ; k < patchIDs_.size() ; k++ )
	{

		int patchID = patchIDs_.at( k );

		scalarField flux = -turbulence_->kappaEff() ().boundaryField()[patchID]
						   * thermo_->T().boundaryField()[patchID].snGrad();

		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}
