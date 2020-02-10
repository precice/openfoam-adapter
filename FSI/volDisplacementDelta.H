#ifndef FSI_VOLDISPLACEMENTDELTA_H
#define FSI_VOLDISPLACEMENTDELTA_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValueFvPatchFields.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads displacementDelta
class volDisplacementDelta : public CouplingDataUser
{

private:

    // DisplacementDelta volVectorField
    Foam::volVectorField * DcellDisplacement_;

public:

    //- Constructor
    volDisplacementDelta
    (
        const Foam::fvMesh& mesh,
        const std::string nameDCellDisplacement
    );

    //- Write the displacement values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double * buffer, const unsigned int dim);

};

}
}

#endif
