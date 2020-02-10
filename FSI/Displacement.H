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

    // Displacement pointVectorField
    Foam::pointVectorField * pointDisplacement_;

public:

    //- Constructor
    Displacement
    (
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement
    );

    //- Write the displacement values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double * buffer, const unsigned int dim);
};

}
}

#endif
