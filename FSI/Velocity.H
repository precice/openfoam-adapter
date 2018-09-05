#ifndef FSI_VELOCITY_H
#define FSI_VELOCITY_H

#include "CouplingDataUser.H"

#include "fvCFD.H"


namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads Velocity
class Velocity : public CouplingDataUser
{

private:

    /* TODO: We probably need to declare some fields here,
    /  see CHT/Temperature.H.
    */
    
    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    // const Foam::fvMesh& mesh_;

    // Get the runTime
    const Foam::Time& runTime_;

    // Displacement
    double * Displ_;

    //- displacement from the previous timestep
    double * DisplOld_;

    // Velocity volVectorField
    Foam::volVectorField * Velocity_;


public:

    //- Constructor
    Velocity
    (
        const Foam::fvMesh& mesh,
        const Foam::Time& runTime,
        const std::string nameVelocity
        /* TODO: We should add any required field names here.
        /  They would need to be vector fields.
        /  See CHT/Temperature.H for details.
        /  Apply any changes also to Velocity.C.
        */
    );

    //- Write the Velocity values into the buffer
    void write(double * buffer);

    //- Read the Velocity values from the buffer
    void read(double * buffer);

};

}
}

#endif
