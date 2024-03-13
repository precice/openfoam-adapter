#ifndef FF_PHI_H
#define FF_PHI_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads the flux phi
class Phi : public CouplingDataUser
{

private:
    //- Phi field
    Foam::surfaceScalarField* phi_;

public:
    //- Constructor
    Phi(
        const Foam::fvMesh& mesh,
        const std::string namePhi);

    //- Write the Phi values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the Phi values from the buffer
    void read(double* buffer, const unsigned int dim);

    bool isLocationTypeSupported(const bool meshConnectivity) const override;

    //- Get the name of the current data field
    std::string getDataName() const override;
};

}
}

#endif
