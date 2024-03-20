#ifndef FF_TEMPERATURE_H
#define FF_TEMPERATURE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads temperature
class Temperature : public CouplingDataUser
{

private:
    //- Temperature field
    Foam::volScalarField* T_;

public:
    //- Constructor
    Temperature(
        const Foam::fvMesh& mesh,
        const std::string nameT);

    //- Write the temperature values into the buffer
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the temperature values from the buffer
    void read(double* buffer, const unsigned int dim);

    bool isLocationTypeSupported(const bool meshConnectivity) const override;

    //- Get the name of the current data field
    std::string getDataName() const override;
};

}
}

#endif
