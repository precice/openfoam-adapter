#ifndef FF_PRESSURE_GRADIENT_H
#define FF_PRESSURE_GRADIENT_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads pressure gradient
class PressureGradient : public CouplingDataUser
{

private:
    //- Pressure field
    Foam::volScalarField* p_;

public:
    //- Constructor
    PressureGradient(
        const Foam::fvMesh& mesh,
        const std::string nameP);

    //- Write the pressure gradient values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the pressure gradient values from the buffer
    void read(double* buffer, const unsigned int dim);

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;
};

}
}

// NOTE: In order to couple another variable, you need to add another
// subclass of the CouplingDataUser. Take this file and PressureGradient.C
// as an example and look for more notes in the Adapter.C and Adapter.H.
// Essentially, the new class needs to implement a constructor (which also
// sets the dataType_) and implement the write() and read() methods.
// Then, you need to create objects of this class in the Adapter.C,
// whenever requested from the configuration file (see notes).

#endif
