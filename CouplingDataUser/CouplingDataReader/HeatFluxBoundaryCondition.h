#ifndef HEATFLUXBOUNDARYCONDITION_H
#define HEATFLUXBOUNDARYCONDITION_H

#include "fvCFD.H"
#include "fixedGradientFvPatchFields.H"
#include "CouplingDataReader.h"

namespace preciceAdapter
{

class HeatFluxBoundaryCondition : public CouplingDataReader
{

protected:

	volScalarField & _T;
	double _k;

public:

	HeatFluxBoundaryCondition( Foam::volScalarField & T, double k );
	void read( double * dataBuffer );

};

}

#endif // HEATFLUXBOUNDARYCONDITION_H
