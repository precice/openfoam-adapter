#ifndef CHT_COMMON_KAPPAEFF_BASE_H
#define CHT_COMMON_KAPPAEFF_BASE_H

#include "fvMesh.H"

namespace preciceAdapter
{
namespace CHT
{

//- Class that extracts the effective conductivity from basic solvers,
//  which do not have a turbulence/thermophysical model, but read
//  the required parameters from the preciceDict dictionary.
class KappaEff_Basic
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Effective conductivity (the same for each cell)
    Foam::dimensionedScalar kappaEff_;

    //- Name of the user-provided conductivity (in the preciceDict)
    const std::string nameKappa_;

public:
    //- Constructor
    KappaEff_Basic(
        const Foam::fvMesh& mesh,
        const std::string nameKappa);

    //- Extract the kappaEff on the specific patch and store it.
    void extract(uint patchID, bool meshConnectivity);

    //- Get the value of kappaEff at cell i (returns the same for every i)
    Foam::scalar getAt(int i);
};

}
}

#endif
