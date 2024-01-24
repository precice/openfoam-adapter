#ifndef FF_ALPHA_H
#define FF_ALPHA_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "cellSet.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads Alpha
class Alpha : public CouplingDataUser
{

private:
    //- Alpha field
    Foam::volScalarField* Alpha_;

public:
    //- Constructor
    Alpha(
        const Foam::fvMesh& mesh,
        const std::string nameAlpha);

    //- Write the Alpha values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the Alpha values from the buffer
    void read(double* buffer, const unsigned int dim);

    bool isLocationTypeSupported(const bool meshConnectivity) const override;

    //- Get the name of the current data field
    std::string getDataName() const override;
};

}
}

#endif
