#ifndef MODULE_HEATFLUX_H
#define MODULE_HEATFLUX_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "cellSet.H"

namespace preciceAdapter
{
namespace MODULE
{

//- Abstract class that writes and reads heat flux.
//  Some parts of it are implemented differently for compressible,
//  incompressible or basic solvers.
class HeatTransfer : public CouplingDataUser
{

protected:
    //- Temperature field
    Foam::volScalarField* T_;

    const Foam::fvMesh& mesh_;

public:
    //- Constructor
    HeatTransfer(const Foam::fvMesh& mesh, const std::string nameT);

    //- Compute heat flux values from the temperature field
    //  and write them into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim) final;

    //- Read heat flux values from the buffer and assign them to
    //  the gradient of the temperature field
    void read(double* buffer, const unsigned int dim) final;

    bool isLocationTypeSupported(const bool meshConnectivity) const final;

    //- Get the name of the current data field
    std::string getDataName() const final;

    //- Destructor
    virtual ~HeatTransfer() {};
};

}
}

#endif
