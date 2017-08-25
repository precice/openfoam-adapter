#include "BuoyantPimpleHeatFluxBoundaryValues.h"

using namespace Foam;

preciceAdapter::BuoyantPimpleHeatFluxBoundaryValues::BuoyantPimpleHeatFluxBoundaryValues( volScalarField * T, basicThermo * thermo, compressible::turbulenceModel * turbulence ) :
	_T( T ),
	_thermo( thermo ),
	_turbulence( turbulence )
{
    _dataType = scalar;
}

void preciceAdapter::BuoyantPimpleHeatFluxBoundaryValues::write( double * dataBuffer )
{

	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		scalarField flux = -_turbulence->kappaEff() ().boundaryField()[patchID]
						   * _thermo->T().boundaryField()[patchID].snGrad();

		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}
