#ifndef CHT_HEATFLUX_COMPRESSIBLE_H
#define CHT_HEATFLUX_COMPRESSIBLE_H

#include "CouplingDataUser.H"
#include "KappaEffective_compressible.H"
#include "KappaEffective_base.H"
#include "HeatFlux_base.H"

namespace preciceAdapter
{
namespace CHT
{

//- Implementation of the HeatFlux for compresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatFlux_Compressible : public HeatFlux
{

protected:
    // Object that extracts the effective conductivity
    // (for compressible turbulent flow solvers)
    KappaEff_Compressible* Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    void extractKappaEff(uint patchID, bool meshConnectivity) final;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    Foam::scalar getKappaEffAt(int i) final;

public:
    //- Constructor
    HeatFlux_Compressible(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Destructor
    ~HeatFlux_Compressible() final;
};

}
}


#endif
