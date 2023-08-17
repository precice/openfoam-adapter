#ifndef CHT_HEATTRANSFERCOEFFICIENT_H
#define CHT_HEATTRANSFERCOEFFICIENT_H

#include "CouplingDataUser.H"
#include "KappaEffective.H"

namespace preciceAdapter
{
namespace CHT
{

//- Abstract class that writes and reads heat transfer coefficient.
//  Some parts of it are implemented differently for compressible,
//  incompressible or basic solvers.
class HeatTransferCoefficient : public CouplingDataUser
{

protected:
    //- Temperature field
    Foam::volScalarField* T_;

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Wrapper for the extract() method of the corresponding KappaEff class
    virtual void extractKappaEff(uint patchID, bool meshConnectivity) = 0;

    //- Wrapper for the getAt() method of the corresponding KappaEff class
    virtual Foam::scalar getKappaEffAt(int i) = 0;

public:
    //- Constructor
    HeatTransferCoefficient(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Write the heat transfer coefficient values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read the heat transfer coefficient values from the buffer
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;

    //- Destructor
    virtual ~HeatTransferCoefficient() {};
};

//- Implementation of the HeatTransferCoefficient for compresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatTransferCoefficient_Compressible : public HeatTransferCoefficient
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
    HeatTransferCoefficient_Compressible(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Destructor
    ~HeatTransferCoefficient_Compressible() final;
};

//- Implementation of the HeatTransferCoefficient for incompresible, turbulent flow solvers
//  which can handle heat transfer.
class HeatTransferCoefficient_Incompressible : public HeatTransferCoefficient
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
    HeatTransferCoefficient_Incompressible(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameRho,
        const std::string nameCp,
        const std::string namePr,
        const std::string nameAlphat);

    //- Destructor
    ~HeatTransferCoefficient_Incompressible() final;
};


//- Implementation of the HeatTransferCoefficient for basic solvers
//  which can handle heat transfer.
class HeatTransferCoefficient_Basic : public HeatTransferCoefficient
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
    HeatTransferCoefficient_Basic(
        const Foam::fvMesh& mesh,
        const std::string nameT,
        const std::string nameKappa);

    //- Destructor
    ~HeatTransferCoefficient_Basic() final;
};

}
}

#endif
