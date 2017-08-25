#include "BuoyantPimpleHeatFluxBoundaryCondition.h"
#include <boost/log/trivial.hpp>

using namespace Foam;

preciceAdapter::BuoyantPimpleHeatFluxBoundaryCondition::BuoyantPimpleHeatFluxBoundaryCondition( volScalarField * T, basicThermo * thermo, compressible::turbulenceModel * turbulence ) :
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

		scalarField kappaEff = _turbulence->kappaEff() ().boundaryField()[patchID];

		fixedGradientFvPatchScalarField & gradientPatch =
			refCast<fixedGradientFvPatchScalarField>( _T->boundaryFieldRef()[patchID] );

		scalarField & gradient = gradientPatch.gradient();

		forAll( gradientPatch, i )
		{
			gradient[i] = dataBuffer[bufferIndex++] / kappaEff[i];
		}

	}
}
