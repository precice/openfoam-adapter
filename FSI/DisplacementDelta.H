#ifndef FSI_DISPLACEMENTDELTA_H
#define FSI_DISPLACEMENTDELTA_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValuePointPatchFields.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads displacement
class DisplacementDelta : public CouplingDataUser
{

private:

    // Displacement pointVectorField
    Foam::pointVectorField * DpointDisplacement_;

public:

    //- Constructor
    DisplacementDelta
    (
        const Foam::fvMesh& mesh,
        const std::string nameDPointDisplacement
    );

    //- Write the displacement values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double * buffer, const unsigned int dim);

};

}
}

#endif
