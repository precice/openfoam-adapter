#ifndef TEMPERATUREBOUNDARYVALUES_H
#define TEMPERATUREBOUNDARYVALUES_H

#include "CouplingDataWriter.H"
#include "fvCFD.H"


namespace preciceAdapter
{

class TemperatureBoundaryValues : public CouplingDataWriter
{

protected:

    Foam::volScalarField * T_;

public:

    TemperatureBoundaryValues(Foam::volScalarField * T);

    void write(double * dataBuffer);

};

}

#endif // TEMPERATUREBOUNDARYVALUES_H
