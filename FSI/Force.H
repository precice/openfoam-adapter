#ifndef FSI_FORCE_H
#define FSI_FORCE_H

#include "ForceBase.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads force
class Force : public ForceBase
{
private:
    //- Force field
    Foam::volVectorField* Force_;

public:
    //- Constructor
    Force(
        const Foam::fvMesh& mesh,
        const std::string solverType);

    //- Write the forces values into the buffer
    void write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the forces values from the buffer
    void read(double* buffer, const unsigned int dim);

    //- Returns the normal vectors multiplied by the face area
    Foam::vectorField getFaceVectors(const unsigned int patchID) const override;

    //- Destructor
    ~Force();
};

}
}

#endif
