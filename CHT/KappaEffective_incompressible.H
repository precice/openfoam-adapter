#ifndef CHT_COMMON_KAPPAEFF_INCOMPRESSIBLE_H
#define CHT_COMMON_KAPPAEFF_INCOMPRESSIBLE_H

#include "KappaEffective_base.H"
#include "incompressible/turbulenceModel/lnInclude/turbulenceModel.H"

namespace preciceAdapter
{
namespace CHT
{

class KappaEff_Incompressible
{

protected:
    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Effective conductivity (it can be different for each cell)
    Foam::scalarField kappaEff_;

    //- Turbulence (and thermo/transport) model.
    //  The effective thermal diffusivity is drawn from it and
    //  used to compute the effective thermal conductivity.
    const Foam::incompressible::turbulenceModel& turbulence_;

    //- Name of the user-provided density (in the preciceDict)
    const std::string nameRho_;

    //- Name of the user-provided heat capacity (in the preciceDict)
    const std::string nameCp_;

    //- Name of the user-provided Prandtl number (in the preciceDict)
    const std::string namePr_;

    //- Name of the turbulent thermal diffusivity field
    const std::string nameAlphat_;

    //- Density
    Foam::dimensionedScalar rho_;

    //- Heat capacity
    Foam::dimensionedScalar Cp_;

    //- Prandtl number
    Foam::dimensionedScalar Pr_;

public:
    //- Constructor
    KappaEff_Incompressible(
        const Foam::fvMesh& mesh,
        const std::string nameRho,
        const std::string nameCp,
        const std::string namePr,
        const std::string nameAlphat);

    //- Extract the kappaEff on the specific patch and store it.
    void extract(uint patchID, bool meshConnectivity);

    //- Get the value of kappaEff at cell i
    Foam::scalar getAt(int i);
};


}
}

#endif
