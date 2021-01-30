#ifndef CHT_SINKTEMPERATURE_H
#define CHT_SINKTEMPERATURE_H

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

//- Class that writes and reads sink temperature.
class SinkTemperature : public CouplingDataUser
{

private:

    //
    Foam::volScalarField & T_;
    const Foam::fvMesh& mesh_;

public:

    //- Constructor
    SinkTemperature
    (
        const Foam::fvMesh& mesh,
        const Foam::word nameT
    );

    //- Destructor
    virtual ~SinkTemperature() = default;

    //- Write the sink temperature values into the buffer
    void write(double *buffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read the sink temperature values from the buffer
    void read(double *buffer, const unsigned int dim) override;

};

}
}

#endif
