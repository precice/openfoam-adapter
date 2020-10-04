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
    Foam::pointVectorField & pointDisplacement_;

public:

    //- Constructor
    Displacement
    (
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement
    );

    virtual ~Displacement() = default;

    //- Write the displacement values into the buffer
    void write(std::vector<double> &dataBuffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read the displacement values from the buffer
    void read(const std::vector<double> &dataBuffer, const unsigned int dim) override;
};

}
}

#endif
