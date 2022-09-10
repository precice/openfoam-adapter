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

    //- Solid force field
    Foam::volVectorField* solidForce_;

public:
    //- Constructor
    Force(
        const Foam::fvMesh& mesh,
        const std::string solverType,
        const std::string nameSolidForce);

    //- Write the forces values into the buffer
    void write(double* buffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read the forces values from the buffer
    void read(double* buffer, const unsigned int dim) override;

    bool isLocationTypeSupported(const bool meshConnectivity) const override;

    //- Get the name of the current data field
    std::string getDataName() const override;

    //- Returns the normal vectors multiplied by the face area
    Foam::tmp<Foam::vectorField> getFaceVectors(const unsigned int patchID) const override;

    //- Destructor
    ~Force();
};

}
}

#endif
