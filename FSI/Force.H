#ifndef FSI_FORCE_H
#define FSI_FORCE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads force
class Force : public CouplingDataUser
{

private:

    /* TODO: We probably need to declare some fields here,
    /  see CHT/Temperature.H.
    */

public:

    //- Constructor
    Force
    (
        const Foam::fvMesh& mesh
        /* TODO: We should add any required field names here.
        /  They would need to be vector fields.
        /  See CHT/Temperature.H for details.
        /  Apply any changes also to Force.C.
        */
    );

    //- Write the displacement values into the buffer
    /* TODO: The buffer should be a vector, how to approach this?
    /  Apply any changes also to Force.C.
    */
    void write(double * buffer);

    //- Read the displacement values from the buffer
    /* TODO: The buffer should be a vector, how to approach this?
    /  Apply any changes also to Force.C.
    */
    void read(double * buffer);

};

}
}

#endif
