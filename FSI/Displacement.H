#ifndef FSI_DISPLACEMENT_H
#define FSI_DISPLACEMENT_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValuePointPatchFields.H"
#include "primitivePatchInterpolation.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads displacement
class Displacement : public CouplingDataUser
{

private:
    // Displacement pointVectorField
    Foam::pointVectorField* pointDisplacement_;

    // cellDisplacement field
    Foam::volVectorField* cellDisplacement_;

    const Foam::fvMesh& mesh_;
    // NOTE: this allocation could be avoided in case we directly write to the
    // Needs to be a pointer since the class disables assignment and copy constructors
    std::vector<const Foam::primitivePatchInterpolation*> interpolationObjects_;

public:
    //- Constructor
    Displacement(
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement,
        const std::string nameCellDisplacement);

    //- Write the displacement values into the buffer
    void write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double* buffer, const unsigned int dim);

    //- We need to initialize the cell-based vector and the interpolation object
    // in case we want to use the faceCenter location for the coupling
    void initialize() override;
};

}
}

#endif
