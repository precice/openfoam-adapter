#ifndef CHT_SINKTEMPERATURE_H
#define CHT_SINKTEMPERATURE_H

#include "CouplingDataUser.H"
#include "mixedFvPatchFields.H"
#include "fvCFD.H"

namespace preciceAdapter
{
namespace CHT
{

//- Class that writes and reads sink temperature.
class SinkTemperature : public CouplingDataUser
{

private:
    //- Temperature field
    Foam::volScalarField* T_;
    const Foam::fvMesh& mesh_;

public:
    //- Constructor
    SinkTemperature(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Write the sink temperature values into the buffer
    void write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the sink temperature values from the buffer
    void read(double* buffer, const unsigned int dim);
};

}
}

#endif
