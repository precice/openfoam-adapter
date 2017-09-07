#ifndef COUPLINGDATAUSER_TEMPERATURE_H
#define COUPLINGDATAUSER_TEMPERATURE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace User
{

class Temperature : public CouplingDataUser
{

private:

    //- Temperature field
    Foam::volScalarField * T_;

public:

    //- Constructor
    Temperature(Foam::volScalarField * T);

    //- Write the temperature values into the buffer
    void write(double * buffer);

    //- Read the temperature values from the buffer
    void read(double * buffer);

};

}
}

#endif
