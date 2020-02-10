#ifndef FSI_VOLDISPLACEMENT_H
#define FSI_VOLDISPLACEMENT_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValueFvPatchFields.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads displacement
class volDisplacement : public CouplingDataUser
{

private:

    // Displacement pointVectorField
    Foam::volVectorField * cellDisplacement_;

public:

    //- Constructor
    volDisplacement
    (
        const Foam::fvMesh& mesh,
        const std::string nameCellDisplacement
    );

    //- Write the displacement values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double * buffer, const unsigned int dim);

};

}
}

#endif
