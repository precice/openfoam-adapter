#ifndef COUPLINGDATAUSER_HEATFLUX_H
#define COUPLINGDATAUSER_HEATFLUX_H

#include "CouplingDataUser.H"
#include "KappaEffective.H"

namespace preciceAdapter
{
namespace User
{

//- Abstract class that writes and reads heat fluxes.
//  Some parts of it are implemented differently for compressible,
//  incompressible or basic solvers.
class HeatFlux : public CouplingDataUser
{

protected:

    //- Temperature field
    Foam::volScalarField * T_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID) = 0;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i) = 0;

public:

    //- Constructor
    HeatFlux(Foam::volScalarField * T);

    //- Compute heat flux values from the temperature field
    //  and write them into the buffer
    virtual void write(double * buffer);

    //- Read heat flux values from the buffer and assign them to
    //  the gradient of the temperature field
    virtual void read(double * buffer);

    //- Destructor
    virtual ~HeatFlux(){};

};

//- Implementation of the HeatFlux for compresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatFlux_Compressible : public HeatFlux
{

protected:

    // Object that extracts the effective conductivity
    // (for compressible turbulent flow solvers)
    KappaEff_Compressible * Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID);

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i);

public:

    //- Constructor
    HeatFlux_Compressible(const Foam::fvMesh& mesh);

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
    KappaEff_Incompressible * Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID);

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i);

public:

    //- Constructor
    HeatFlux_Incompressible(const Foam::fvMesh& mesh);

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
    KappaEff_Basic * Kappa_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID);

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i);

public:

    //- Constructor
    HeatFlux_Basic(const Foam::fvMesh& mesh);

    //- Destructor
    virtual ~HeatFlux_Basic();

};

}
}


#endif
