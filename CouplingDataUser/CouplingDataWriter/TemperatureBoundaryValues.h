#ifndef TEMPERATUREBOUNDARYVALUES_H
#define TEMPERATUREBOUNDARYVALUES_H

#include "CouplingDataWriter.h"
#include "fvCFD.H"


namespace preciceAdapter
{

class TemperatureBoundaryValues : public CouplingDataWriter
{

protected:

	Foam::volScalarField * _T;

public:

	TemperatureBoundaryValues( Foam::volScalarField * T );
	void write( double * dataBuffer );

};

}

#endif // TEMPERATUREBOUNDARYVALUES_H
