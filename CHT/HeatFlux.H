#ifndef CHT_HEATFLUX_H
#define CHT_HEATFLUX_H

#include "CouplingDataUser.H"
#include "KappaEffective.H"

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
    virtual void write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read heat flux values from the buffer and assign them to
    //  the gradient of the temperature field
    virtual void read(double* buffer, const unsigned int dim);

    //- Destructor
    virtual ~HeatFlux() {};
};

//- Implementation of the HeatFlux for compresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatFlux_Compressible : public HeatFlux
{

protected:
    // Object that extracts the effective conductivity
    // (for compressible turbulent flow solvers)
    KappaEff_Compressible* Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID, bool meshConnectivity);

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i);

public:
    //- Constructor
    HeatFlux_Compressible(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Destructor
    virtual ~HeatFlux_Compressible();
};

//- Implementation of the HeatFlux for incompresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatFlux_Incompressible : public HeatFlux
{

protected:
    // Object that computes the effective conductivity
    // (for incompressible turbulent flow solvers)
    KappaEff_Incompressible* Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID, bool meshConnectivity);

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i);

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
    virtual ~HeatFlux_Incompressible();
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
    virtual void extractKappaEff(uint patchID, bool meshConnectivity);

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i);

public:
    //- Constructor
    HeatFlux_Basic(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameKappa);

    //- Destructor
    virtual ~HeatFlux_Basic();
};

}
}


#endif
