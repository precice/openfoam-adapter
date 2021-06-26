#ifndef VOLUMECOUPLING_FLUID_VELOCITY_H
#define VOLUMECOUPLING_FLUID_VELOCITY_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace Volume_Coupling
{

//- Class that writes and reads Fluid_Velocity
class Fluid_Velocity : public CouplingDataUser
{

private:

    //- Velocity field
    Foam::volVectorField * Fluid_Velocity_;


public:

    //- Constructor
 Fluid_Velocity
    (
        const Foam::fvMesh& mesh,
        const std::string nameU
    );

    //- Write the displacement values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double * buffer, const unsigned int dim);

    //- Destructor
    ~Fluid_Velocity();


};

}
}

#endif
