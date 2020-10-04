#ifndef CHT_HEATFLUX_H
#define CHT_HEATFLUX_H

#include <string>
#include <vector>

#include "CouplingDataUser.H"

#include "fvMesh.H"
#include "volFieldsFwd.H"

namespace preciceAdapter
{
namespace CHT
{

class HeatFlux : public CouplingDataUser
{

private:

    //
    Foam::volScalarField & T_;
    const Foam::fvMesh& mesh_;

public:

    //- Constructor
    HeatFlux(const Foam::fvMesh& mesh, const std::string nameT);

    //- Destructor
    virtual ~HeatFlux() = default;

    //- Compute heat flux values from the temperature field
    //  and write them into the buffer
    void write(std::vector<double> &buffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read heat flux values from the buffer and assign them to
    //  the gradient of the temperature field
    void read(const std::vector<double> &buffer, const unsigned int dim) override;

};

}
}


#endif
