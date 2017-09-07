#include "BuoyantPimpleHeatFluxBoundaryCondition.h"
#include <boost/log/trivial.hpp>

using namespace Foam;

preciceAdapter::BuoyantPimpleHeatFluxBoundaryCondition::BuoyantPimpleHeatFluxBoundaryCondition( volScalarField * T, basicThermo * thermo, compressible::turbulenceModel * turbulence ) :
	T_( T ),
	thermo_( thermo ),
	turbulence_( turbulence )
{
	dataType_ = scalar;
}

void preciceAdapter::BuoyantPimpleHeatFluxBoundaryCondition::read( double * dataBuffer )
{

	// BOOST_LOG_TRIVIAL( info ) << "Setting heat flux boundary condition";

	int bufferIndex = 0;

	for( uint k = 0 ; k < patchIDs_.size() ; k++ )
	{

		int patchID = patchIDs_.at( k );

		scalarField kappaEff = turbulence_->kappaEff() ().boundaryField()[patchID];

		fixedGradientFvPatchScalarField & gradientPatch =
			refCast<fixedGradientFvPatchScalarField>( T_->boundaryFieldRef()[patchID] );

		scalarField & gradient = gradientPatch.gradient();

		forAll( gradientPatch, i )
		{
			gradient[i] = dataBuffer[bufferIndex++] / kappaEff[i];
		}

	}
}
