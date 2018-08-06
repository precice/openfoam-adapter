#ifndef FSI_DISPLACEMENT_H
#define FSI_DISPLACEMENT_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValuePointPatchFields.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads displacement
class Displacement : public CouplingDataUser
{

private:

    /* TODO: We probably need to declare some fields here,
    /  see CHT/Temperature.H.
    */
    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;

    //- OpenFOAM Time object (we need to access the timestep to compute the pointMotionU)
    const Foam::Time& runTime_;

    //- Displacements
    vectorField Displ_;

    //- Displacements from the previous timestep
    //  We need them to compute and set the pointMotionU
    vectorField DisplOld_;

    // TODO: We have this information already in the Interface,
    // implement it in a better way.
    int numDataLocations_;

public:

    //- Constructor
    Displacement
    (
        const Foam::fvMesh& mesh,
        const Foam::Time& runTime
        /* TODO: We should add any required field names here.
        /  They would need to be vector fields.
        /  See CHT/Temperature.H for details.
        /  Apply any changes also to Displacement.C.
        */
    );

    //- Write the displacement values into the buffer
    void write(double * buffer);

    //- Read the displacement values from the buffer
    void read(double * buffer);

};

}
}

#endif
