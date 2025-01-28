#ifndef MODULE_HEATFLUX_H
#define MODULE_HEATFLUX_H

#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "cellSet.H"

namespace preciceAdapter
{
namespace MODULE
{

class Temperature : public CouplingDataUser
{
protected:
    Foam::volScalarField* T_;
    const Foam::fvMesh& mesh_;

public:
    Temperature(const Foam::fvMesh& mesh, const std::string nameT);

    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);
    void read(double* buffer, const unsigned int dim);
    bool isLocationTypeSupported(const bool meshConnectivity) const;
    std::string getDataName() const;

    virtual ~Temperature() {};
};

class HeatFlux : public CouplingDataUser
{
protected:
    Foam::volScalarField* T_;
    const Foam::fvMesh& mesh_;

public:
    HeatFlux(const Foam::fvMesh& mesh, const std::string nameT);

    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);
    void read(double* buffer, const unsigned int dim);
    bool isLocationTypeSupported(const bool meshConnectivity) const;
    std::string getDataName() const;

    virtual ~HeatFlux() {};
};

}
}

#endif
