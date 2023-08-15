#ifndef CHT_HEATFLUX_BASE_H
#define CHT_HEATFLUX_BASE_H

#include "CouplingDataUser.H"
#include "KappaEffective_base.H"

namespace preciceAdapter
{
namespace CHT
{

//- Abstract class that writes and reads heat flux.
//  Some parts of it are implemented differently for compressible,
//  incompressible or basic solvers.
class HeatFlux : public CouplingDataUser
{

protected:
    //- Temperature field
    Foam::volScalarField* T_;

    const Foam::fvMesh& mesh_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID, bool meshConnectivity) = 0;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i) = 0;

public:
    //- Constructor
    HeatFlux(const Foam::fvMesh& mesh, const std::string nameT);

    //- Compute heat flux values from the temperature field
    //  and write them into the buffer
    void write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read heat flux values from the buffer and assign them to
    //  the gradient of the temperature field
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;

    //- Destructor
    virtual ~HeatFlux() {};
};

//- Implementation of the HeatFlux for basic solvers
//  which can handle heat transfer.
class HeatFlux_Basic : public HeatFlux
{

protected:
    //- Object that extracts the effective conductivity
    // (for basic solvers)
    KappaEff_Basic* Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    void extractKappaEff(uint patchID, bool meshConnectivity) final;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    Foam::scalar getKappaEffAt(int i) final;

public:
    //- Constructor
    HeatFlux_Basic(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameKappa);

    //- Destructor
    ~HeatFlux_Basic() final;
};

}
}


#endif
