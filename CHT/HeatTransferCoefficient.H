#ifndef CHT_HEATTRANSFERCOEFFICIENT_H
#define CHT_HEATTRANSFERCOEFFICIENT_H

#include "CouplingDataUser.H"

#include "fvMesh.H"
#include "volFieldsFwd.H"

// forward declaration of foam space
namespace Foam
{
class word;
}

namespace preciceAdapter
{
namespace CHT
{

class HeatTransferCoefficient : public CouplingDataUser
{

private:

    //
    Foam::volScalarField & T_;
    const Foam::fvMesh& mesh_;

public:

    //- Constructor
    HeatTransferCoefficient
    (
        const Foam::fvMesh& mesh,
        const Foam::word nameT
    );

    //- Destructor
    virtual ~HeatTransferCoefficient() = default;

    //- Write the heat transfer coefficient values into the buffer
    void write(double *buffer, bool meshConnectivity, const unsigned int dim) override;

    //- Read the heat transfer coefficient values from the buffer
    void read(double *buffer, const unsigned int dim) override;

};

}
}

#endif
