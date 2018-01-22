#ifndef FLUIDS_VELOCITY_H
#define FLUIDS_VELOCITY_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace Fluids
{

//- Class that writes and reads temperature.
class Velocity : public CouplingDataUser
{

private:

    //- Velocity field
    Foam::volVectorField * U_;

public:

    //- Constructor
    Velocity
    (
        const Foam::fvMesh& mesh,
        const std::string nameT
    );

    //- Write the temperature values into the buffer
    void write(double * buffer);

    //- Read the temperature values from the buffer
    void read(double * buffer);

};

}
}

// NOTE: In order to couple another variable, you need to add another
// subclass of the CouplingDataUser. Take this file and Temperature.C
// as an example and look for more notes in the Adapter.C and Adapter.H.
// Essentially, the new class needs to implement a constructor (which also
// sets the dataType_) and implement the write() and read() methods.
// Then, you need to create objects of this class in the Adapter.C,
// whenever requested from the configuration file (see notes).

#endif
