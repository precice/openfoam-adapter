#ifndef FSI_FORCE_H
#define FSI_FORCE_H

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
class Force : public CouplingDataUser
{

private:

    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;
    
    const std::string solverType_;

    //- Force field
    Foam::volVectorField * Force_;

    // Needed for inheritance to the Stress data field to avoid creating the Force
    // field in case it is not required
    const bool force_field_created;

protected:
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

    //- Constructor
    Force
    (
        const Foam::fvMesh& mesh,
        const std::string solverType
    );

    //- Destructor
    virtual ~Force();

    //- Write the forces values into the buffer
    void write(std::vector<double> &dataBuffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read the forces values from the buffer
    void read(const std::vector<double> &dataBuffer, const unsigned int dim) override;

};

}
}

#endif
