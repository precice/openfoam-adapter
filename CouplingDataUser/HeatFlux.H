#ifndef COUPLINGDATAUSER_HEATFLUX_H
#define COUPLINGDATAUSER_HEATFLUX_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedGradientFvPatchFields.H"

namespace preciceAdapter
{
namespace User
{

class HeatFlux : public CouplingDataUser
{

private:

    //- Temperature field
    Foam::volScalarField * T_;

    //- TODO Thermal conductivity
    double k_;

public:

    //- Constructor
    HeatFlux(Foam::volScalarField * T, double k);

    //- Compute heat flux values from the temperature field
    //  and write them into the buffer
    void write(double * buffer);

    //- Read heat flux values from the buffer and assign them to
    //  the gradient of the temperature field
    void read(double * buffer);

};

}
}


#endif
