#ifndef CHT_COMMON_KAPPAEFF_H
#define CHT_COMMON_KAPPAEFF_H

#include "turbulentFluidThermoModel.H"
#include "turbulentTransportModel.H"

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

    //- Turbulence (and thermo/transport) model, from which
    //  kappaEff is drawn directly.
    const Foam::compressible::turbulenceModel& turbulence_;

public:
    //- Constructor
    KappaEff_Compressible(const Foam::fvMesh& mesh);

    //- Extract the kappaEff on the specific patch and store it.
    void extract(uint patchID, bool meshConnectivity);

    //- Get the value of kappaEff at cell i
    Foam::scalar getAt(int i);
};

//- Class that extracts the effective conductivity from incompressible
//  turbulent flow solvers.
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
