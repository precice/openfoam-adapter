#ifndef BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H
#define BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "basicThermo.H"
#include "turbulentFluidThermoModel.H"
#include "CouplingDataWriter.h"

namespace preciceAdapter
{

class BuoyantPimpleHeatFluxBoundaryValues : public CouplingDataWriter
{

protected:

	volScalarField * _T;
	basicThermo * _thermo;
	Foam::compressible::turbulenceModel * _turbulence;

public:

	BuoyantPimpleHeatFluxBoundaryValues( volScalarField * T,
										 Foam::basicThermo * thermo,
										 Foam::compressible::turbulenceModel * turbulence );

	void write( double * dataBuffer );

};

}

#endif // BUOYANTPIMPLEHEATFLUXBOUNDARYVALUES_H
