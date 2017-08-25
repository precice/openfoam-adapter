#include "BuoyantPimpleHeatFluxBoundaryCondition.h"
#include <boost/log/trivial.hpp>


preciceAdapter::BuoyantPimpleHeatFluxBoundaryCondition::BuoyantPimpleHeatFluxBoundaryCondition( Foam::volScalarField * T, Foam::basicThermo * thermo, Foam::compressible::turbulenceModel * turbulence ) :
	_T( T ),
	_thermo( thermo ),
	_turbulence( turbulence )
{
	_dataType = scalar;
}

void preciceAdapter::BuoyantPimpleHeatFluxBoundaryCondition::read( double * dataBuffer )
{

	// BOOST_LOG_TRIVIAL( info ) << "Setting heat flux boundary condition";

	int bufferIndex = 0;

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{

		int patchID = _patchIDs.at( k );

		Foam::scalarField kappaEff = _turbulence->kappaEff() ().boundaryField()[patchID];

		Foam::fixedGradientFvPatchScalarField & gradientPatch =
			Foam::refCast<Foam::fixedGradientFvPatchScalarField>( _T->boundaryFieldRef()[patchID] );

		Foam::scalarField & gradient = gradientPatch.gradient();

		forAll( gradientPatch, i )
		{
			gradient[i] = dataBuffer[bufferIndex++] / kappaEff[i];
		}

	}
}
