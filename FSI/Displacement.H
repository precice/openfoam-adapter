#ifndef FSI_DISPLACEMENT_H
#define FSI_DISPLACEMENT_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValuePointPatchFields.H"

// Add classes here?
#include "pointFields.H"
#include "vectorField.H"



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
   
    // Displacement pointVectorField
    Foam::pointVectorField * pointDisplacement_;

public:
    
    //- Constructor
    Displacement
    (
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement
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

    // TODO. needs destructor?
    //- Destructor???
    // ~Displacment();
};

}
}

#endif
