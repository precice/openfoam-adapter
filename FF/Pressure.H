#ifndef FF_PRESSURE_H
#define FF_PRESSURE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads pressure
class Pressure : public CouplingDataUser
{

private:
    //- Pressure field
    Foam::volScalarField* p_;

public:
    //- Constructor
    Pressure(
        const Foam::fvMesh& mesh,
        const std::string nameP);

    //- Write the pressure values into the buffer
    void write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the pressure values from the buffer
    void read(double* buffer, const unsigned int dim);
};

}
}

// NOTE: In order to couple another variable, you need to add another
// subclass of the CouplingDataUser. Take this file and Pressure.C
// as an example and look for more notes in the Adapter.C and Adapter.H.
// Essentially, the new class needs to implement a constructor (which also
// sets the dataType_) and implement the write() and read() methods.
// Then, you need to create objects of this class in the Adapter.C,
// whenever requested from the configuration file (see notes).

#endif
