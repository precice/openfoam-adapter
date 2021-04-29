#ifndef FORCEBASE_H
#define FORCEBASE_H


#include "CouplingDataUser.H"

#include "fvCFD.H"

#include "pointFields.H"
#include "vectorField.H"
#include "immiscibleIncompressibleTwoPhaseMixture.H"
#include "turbulentFluidThermoModel.H"
#include "turbulentTransportModel.H"

namespace preciceAdapter
{
namespace FSI
{

//- Class that writes and reads force
class ForceBase : public CouplingDataUser
{
protected:
    //- Stress tensor (see the OpenFOAM "Forces" function object)
    Foam::tmp<Foam::volSymmTensorField> devRhoReff() const;

    Foam::tmp<Foam::volScalarField> rho() const;

    Foam::tmp<Foam::volScalarField> mu() const;

    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;

    const std::string solverType_;

public:
    //- Constructor
    ForceBase(
        const Foam::fvMesh& mesh,
        const std::string solverType);

    void writeToBuffer(double* buffer,
                       Foam::volVectorField& forceField,
                       const unsigned int dim) const;

    void readFromBuffer(double* buffer) const;

    virtual Foam::vectorField getFaceVectors(const unsigned int patchID) const = 0;
};
}
}

#endif // FORCEBASE_H
