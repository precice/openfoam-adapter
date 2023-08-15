#ifndef CHT_COMMON_KAPPAEFF_COMPRESSIBLE_H
#define CHT_COMMON_KAPPAEFF_COMPRESSIBLE_H

#include "KappaEffective_base.H"
#include "compressible/turbulenceModel/lnInclude/turbulenceModel.H"

namespace preciceAdapter
{
namespace CHT
{

//- Class that extracts the effective conductivity from compressible
//  turbulent flow solvers.
class KappaEff_Compressible
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Effective conductivity (it can be different for each cell)
    Foam::scalarField kappaEff_;

    //- Turbulence (and thermo/transport) model
    const Foam::compressible::turbulenceModel& turbulence_;
    
public:
    //- Constructor
    KappaEff_Compressible(const Foam::fvMesh& mesh);

    //- Extract the kappaEff on the specific patch and store it.
    void extract(uint patchID, bool meshConnectivity);

    //- Get the value of kappaEff at cell i
    Foam::scalar getAt(int i);
};


}
}

#endif
