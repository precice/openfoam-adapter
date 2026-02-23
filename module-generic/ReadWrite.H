#ifndef MODULE_ReadWrite_H
#define MODULE_ReadWrite_H

#include "Interface.H"
#include "CouplingDataUser.H"

#include "fvCFD.H"
#include "cellSet.H"

namespace preciceAdapter
{
namespace Generic
{

class ScalarFieldCoupler : public CouplingDataUser // One CouplingDataUser for each field
{
protected:
    Foam::volScalarField* scalarField_;
    const Foam::fvMesh& mesh_;
    const preciceAdapter::FieldConfig& fieldConfig_;

public:
    ScalarFieldCoupler(const Foam::fvMesh& mesh, const preciceAdapter::FieldConfig& fieldConfig);

    void initialize();
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);
    void read(double* buffer, const unsigned int dim);
    bool isLocationTypeSupported(const bool meshConnectivity) const;
    std::string getDataName() const;

    virtual ~ScalarFieldCoupler() {};
};

class VectorFieldCoupler : public CouplingDataUser
{
protected:
    Foam::volVectorField* vectorField_;
    const Foam::fvMesh& mesh_;
    const preciceAdapter::FieldConfig& fieldConfig_;

public:
    VectorFieldCoupler(const Foam::fvMesh& mesh, const preciceAdapter::FieldConfig& fieldConfig);

    void initialize();
    std::size_t write(double* buffer, bool meshConnectivity, const unsigned int dim);
    void read(double* buffer, const unsigned int dim);
    bool isLocationTypeSupported(const bool meshConnectivity) const;
    std::string getDataName() const;

    virtual ~VectorFieldCoupler() {};
};

}
}

#endif
