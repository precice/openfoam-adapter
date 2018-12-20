#ifndef FSI_FORCE_H
#define FSI_FORCE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads force
class Force : public CouplingDataUser
{

private:

    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;
    
    //- Force field
    Foam::volVectorField * Force_;


    //- Stress tensor (see the OpenFOAM "Forces" function object)
    Foam::tmp<Foam::volSymmTensorField> devRhoReff(dimensionedScalar rho) const;

public:

    //- Constructor
    Force
    (
        const Foam::fvMesh& mesh,
        const fileName& timeName // We create an IOobject and we need the time directory
    );

    //- Write the displacement values into the buffer
    void write(double * buffer);

    //- Read the displacement values from the buffer
    void read(double * buffer);

    //- Destructor
    ~Force();

};

}
}

#endif
