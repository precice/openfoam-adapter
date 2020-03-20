#ifndef FSI_FORCE_H
#define FSI_FORCE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

// TEMPORARY
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
class Force : public CouplingDataUser
{

private:

    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;
    
    const std::string solverType_;

    //- Force field
    Foam::volVectorField * Force_;


    //- Stress tensor (see the OpenFOAM "Forces" function object)
    Foam::tmp<Foam::volSymmTensorField> devRhoReff() const;
    
    Foam::tmp<Foam::volScalarField> rho() const;
    
    Foam::tmp<Foam::volScalarField> mu() const;


public:

    //- Constructor
    Force
    (
        const Foam::fvMesh& mesh,
        const fileName& timeName,
        const std::string solverType
        // We create an IOobject and we need the time directory
        /* TODO: We should add any required field names here.
        /  They would need to be vector fields.
        /  See CHT/Temperature.H for details.
        /  Apply any changes also to Force.C.
        */
    );

    //- Write the displacement values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the displacement values from the buffer
    void read(double * buffer, const unsigned int dim);

    //- Destructor
    ~Force();

};

}
}

#endif
