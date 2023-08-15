#ifndef CHT_HEATFLUX_INCOMPRESSIBLE_H
#define CHT_HEATFLUX_INCOMPRESSIBLE_H

#include "CouplingDataUser.H"
#include "KappaEffective_incompressible.H"
#include "KappaEffective_base.H"
#include "HeatFlux_base.H"

namespace preciceAdapter
{
namespace CHT
{

//- Implementation of the HeatFlux for incompresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatFlux_Incompressible : public HeatFlux
{

protected:
    // Object that computes the effective conductivity
    // (for incompressible turbulent flow solvers)
    KappaEff_Incompressible* Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    void extractKappaEff(uint patchID, bool meshConnectivity) final;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    Foam::scalar getKappaEffAt(int i) final;

public:
    //- Constructor
    HeatFlux_Incompressible(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameRho,
        const std::string nameCp,
        const std::string namePr,
        const std::string nameAlphat);

    //- Destructor
    ~HeatFlux_Incompressible() final;
};

}
}


#endif
