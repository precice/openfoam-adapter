#ifndef FSI_STRESS_H
#define FSI_STRESS_H

#include "CouplingDataUser.H"
#include "Force.H"

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

//- Class that writes and reads stress [N/m^2]:
// This is essentially a force (in spatial coordinates) scaled by the deformed
// cell face. Thus, a consistent quantity. Calculation concept has been copied
// from the force module, but the scaled version here is commonly used in FEM
// applications.
class Stress : public Force {

private:

    //- OpenFOAM fvMesh object (we need to access the objects' registry multiple times)
    const Foam::fvMesh& mesh_;
    
    const std::string solverType_;

    //- Stress field
    Foam::volVectorField * Stress_;


public:

    //- Constructor
    Stress
    (
        const Foam::fvMesh& mesh,
        const fileName& timeName,
        const std::string solverType
        // We create an IOobject and we need the time directory
        /* TODO: We should add any required field names here.
        */
    );

    //- Write the stress values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the stress values from the buffer
    void read(double * buffer, const unsigned int dim);

    //- Destructor
    ~Stress();
};
}
}

#endif
