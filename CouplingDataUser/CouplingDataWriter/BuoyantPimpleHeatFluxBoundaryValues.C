#include "BuoyantPimpleHeatFluxBoundaryValues.h"


preciceAdapter::BuoyantPimpleHeatFluxBoundaryValues::BuoyantPimpleHeatFluxBoundaryValues( Foam::volScalarField * T, Foam::basicThermo * thermo, Foam::compressible::turbulenceModel * turbulence ) :
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

		Foam::scalarField flux = -_turbulence->kappaEff() ().boundaryField()[patchID]
						   * _thermo->T().boundaryField()[patchID].snGrad();

		forAll( flux, i )
		{
			dataBuffer[bufferIndex++] = flux[i];
		}

	}
}
