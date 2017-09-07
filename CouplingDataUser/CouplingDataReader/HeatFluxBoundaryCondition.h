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

	volScalarField * T_;
	double k_;

public:

	HeatFluxBoundaryCondition( Foam::volScalarField * T, double k );
	void read( double * dataBuffer );

};

}

#endif // HEATFLUXBOUNDARYCONDITION_H
