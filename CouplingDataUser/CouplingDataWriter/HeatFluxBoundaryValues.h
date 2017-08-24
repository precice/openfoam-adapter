#ifndef HEATFLUXBOUNDARYVALUES_H
#define HEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "CouplingDataWriter.h"

namespace preciceAdapter
{

class HeatFluxBoundaryValues : public CouplingDataWriter
{

protected:

	Foam::volScalarField & _T;
	double _k;

public:

	HeatFluxBoundaryValues( Foam::volScalarField & T, double k );
	void write( double * dataBuffer );

};

}

#endif // HEATFLUXBOUNDARYVALUES_H
