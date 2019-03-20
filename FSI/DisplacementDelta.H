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
    Foam::pointVectorField * pointDisplacement_;

public:

    //- Constructor
    DisplacementDelta
    (
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement
    );

    //- Write the displacementDelta values into the buffer
    void write(double * buffer, bool provideMeshConnectivity);

    //- Read the displacementDelta values from the buffer
    void read(double * buffer);

};

}
}

#endif
