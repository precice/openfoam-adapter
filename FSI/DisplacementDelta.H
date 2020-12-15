#ifndef FSI_DISPLACEMENTDELTA_H
#define FSI_DISPLACEMENTDELTA_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "fixedValuePointPatchFields.H"
#include "primitivePatchInterpolation.H"

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

    const Foam::fvMesh& mesh_;

    // vector field for cell displacement on each interface patch
    // NOTE: both allocation could be avoided in case we directly write to the
    // cellDisplacement field
    std::vector<Foam::vectorField> boundaryCellDisplacement_;
    // Needs to be a pointer since the class disables assignment and copy constructors
    std::vector<Foam::primitivePatchInterpolation *> interpolationObjects_;

  public:

    //- Constructor
    DisplacementDelta
    (
        const Foam::fvMesh& mesh,
        const std::string namePointDisplacement
    );

    //- Write the displacementDelta values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacementDelta values from the buffer
    void read(double * buffer, const unsigned int dim);

    //- We need to initialize the cell-based vector and the interpolation object
    // in case we want to use the faceCenter location for the coupling
    void initialize() override;
};

}
}

#endif
