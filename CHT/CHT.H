#ifndef CHT_H
#define CHT_H

#include "Interface.H"

#include "CHT/Temperature.H"
#include "CHT/HeatFlux.H"
#include "CHT/SinkTemperature.H"
#include "CHT/HeatTransferCoefficient.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace CHT
{

class ConjugateHeatTransfer
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- Solver type
    std::string solverType_ = "none";

    //- Name of the transportProperties dictionary
    std::string nameTransportProperties_;

    //- Name of the temperature field
    std::string nameT_;

    //- Name of the thermal conductivity for a basic solver
    std::string nameKappa_;

    //- Name of the density for an incompressible solver
    std::string nameRho_;

    //- Name of the specific heat for an incompressible solver
    std::string nameCp_;

    //- Name of the Prandtl number for an incompressible solver
    std::string namePr_;

    //- Name of the turbulent thermal diffusivity field for an incompressible solver
    std::string nameAlphat_;

public:

    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

    //- Determine the solver type
    std::string determineSolverType();

    //- Configure
    void configure();

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);

    //- Set the solver type
    void setSolverType(const std::string solverType);

    //- Set the name of the temperature field
    void setNameT(const std::string nameT);

    //- Set the name of the transportProperties dictionary
    void setNameTransportProperties(const std::string nameTransportProperties);

    //- Set the name of the user-provided thermal conductivity k
    void setNameKappa(const std::string nameKappa);

    //- Set the name of the user-provided density rho
    void setNameRho(const std::string nameRho);

    //- Set the name of the user-provided heat capacity Cp
    void setNameCp(const std::string nameCp);

    //- Set the name of the user-provided Prandtl number Pr
    void setNamePr(const std::string namePr);

    //- Set the name of the turbulent thermal diffusivity
    void setNameAlphat(const std::string nameAlphat);
};

}
}

#endif
