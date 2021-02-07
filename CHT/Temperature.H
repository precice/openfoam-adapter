#ifndef CHT_TEMPERATURE_H
#define CHT_TEMPERATURE_H

#include "CouplingDataUser.H"

#include "fvMesh.H"
#include "volFieldsFwd.H"

// forward declaration of foam space
namespace Foam
{
class word;
}

namespace preciceAdapter
{
namespace CHT
{

//- Class that writes and reads temperature.
class Temperature : public CouplingDataUser
{

private:

    //
    Foam::volScalarField & T_;
    const Foam::fvMesh& mesh_;

public:

    //- Constructor
    Temperature
    (
        const Foam::fvMesh& mesh,
        const Foam::word nameT
    );

    //- Destructor
    virtual ~Temperature() = default;

    //- Write the temperature values into the buffer
    void write(double *buffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read the temperature values from the buffer
    void read(double *buffer, const unsigned int dim) override;

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
