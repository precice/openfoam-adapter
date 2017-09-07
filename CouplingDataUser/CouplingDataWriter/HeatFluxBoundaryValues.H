#ifndef HEATFLUXBOUNDARYVALUES_H
#define HEATFLUXBOUNDARYVALUES_H

#include "fvCFD.H"
#include "CouplingDataWriter.H"

namespace preciceAdapter
{

class HeatFluxBoundaryValues : public CouplingDataWriter
{

protected:

    Foam::volScalarField * T_;

    double k_;

public:

    HeatFluxBoundaryValues(Foam::volScalarField * T, double k);

    void write(double * dataBuffer);

};

}

#endif // HEATFLUXBOUNDARYVALUES_H
