#ifndef FF_HU_SWE_H
#define FF_HU_SWE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads hu_SWE
class hu_SWE : public CouplingDataUser
{

private:

    //- hu_SWE field
    Foam::volVectorField * U_;


public:

    //- Constructor
    hu_SWE
    (
        const Foam::fvMesh& mesh,
        const std::string nameU
    );

    //- Write the hu_SWE values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the hu_SWE values from the buffer
    void read(double * buffer, const unsigned int dim);

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
