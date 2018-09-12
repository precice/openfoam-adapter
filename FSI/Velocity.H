#ifndef FSI_VELOCITY_H
#define FSI_VELOCITY_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "primitivePatchInterpolation.H"

#include "fixedValuePointPatchFields.H"
#include "fvPatchFields.H"
#include "fixedValueFvPatchFields.H"
#include "fixedValueFvPatchFields.H"

// #include "Displacement.H"

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
    const Foam::fvMesh& mesh_;

    // Get the runTime
    const Foam::Time& runTime_;
    
    // set up te displacement at the faces.
    Foam::volVectorField * faceDisplacement_;
    Foam::volVectorField * faceDisplacementOld_;

    // object to save the times of faceDisplacement
    Foam::scalar time_;
    Foam::scalar timeOld_;

    // Velocity volVectorField
    Foam::volVectorField * velocity_;

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

    // TODO. needs destructor?
    //- Destructor???
    // ~Velocity();

};

}
}

#endif
