#ifndef FSI_VELOCITY_H
#define FSI_VELOCITY_H

#include "CouplingDataUser.H"

#include "OpenFOAMCompatibility.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that reads the rigid-body airframe velocity into the U field
//  on the coupling interface (the airframe surface).
class Velocity : public CouplingDataUser
{

private:
    //- Velocity field (non-owning pointer to U)
    Foam::volVectorField* U_;

    //- OpenFOAM mesh
    const Foam::fvMesh& mesh_;

public:
    //- Constructor
    Velocity(const Foam::fvMesh& mesh);

    //- Write the velocity values into the buffer (not used, the
    //  velocity is only read from preCICE)
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read the velocity values from the buffer
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;
};

}
}

#endif