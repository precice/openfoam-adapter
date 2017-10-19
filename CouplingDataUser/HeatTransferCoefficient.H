#ifndef COUPLINGDATAUSER_HEATTRANSFERCOEFFICIENT_H
#define COUPLINGDATAUSER_HEATTRANSFERCOEFFICIENT_H

#include "CouplingDataUser.H"
#include "KappaEffective.H"

namespace preciceAdapter
{
namespace User
{

//- Abstract class that writes and reads heat transfer coefficient.
//  Some parts of it are implemented differently for compressible,
//  incompressible or basic solvers.
class HeatTransferCoefficient : public CouplingDataUser
{

protected:

    //- Temperature field
    Foam::volScalarField * T_;

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID) = 0;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i) = 0;

public:

    //- Constructor
    HeatTransferCoefficient(const Foam::fvMesh& mesh);

    //- Write the heat transfer coefficient values into the buffer
    virtual void write(double * buffer);

    //- Read the heat transfer coefficient values from the buffer
    virtual void read(double * buffer);

    //- Destructor
    virtual ~HeatTransferCoefficient(){};

};

//- Implementation of the HeatTransferCoefficient for compresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatTransferCoefficient_Compressible : public HeatTransferCoefficient
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
    HeatTransferCoefficient_Compressible(const Foam::fvMesh& mesh);

    //- Destructor
    virtual ~HeatTransferCoefficient_Compressible();

};

//- Implementation of the HeatTransferCoefficient for incompresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatTransferCoefficient_Incompressible : public HeatTransferCoefficient
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
    HeatTransferCoefficient_Incompressible(const Foam::fvMesh& mesh);

    //- Destructor
    virtual ~HeatTransferCoefficient_Incompressible();

};


//- Implementation of the HeatTransferCoefficient for basic solvers
//  which can handle heat transfer.
class HeatTransferCoefficient_Basic : public HeatTransferCoefficient
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
    HeatTransferCoefficient_Basic(const Foam::fvMesh& mesh);

    //- Destructor
    virtual ~HeatTransferCoefficient_Basic();

};

}
}

#endif
