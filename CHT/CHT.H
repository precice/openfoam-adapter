#ifndef CHT_H
#define CHT_H

#include <optional>

// forward declaration of foam space
namespace foam
{

class fvMesh;
class IOdictionary;

}

namespace preciceAdapter
{

// forward declaration
class Interface;

namespace CHT
{

// Helper for parameter names.
struct CHTParameterNames
{
    //- Name of the temperature field
    std::string T = "T";

    // //- Name of the thermal conductivity for a basic solver
    // std::string Kappa = "k";

    // //- Name of the density for an incompressible solver
    // std::string Rho = "rho";

    // //- Name of the heat capacity for an incompressible solver
    // std::string Cp = "Cp";

    // //- Name of the Prandtl number for an incompressible solver
    // std::string Pr = "Pr";

    // //- Name of the turbulent thermal diffusivity field for an incompressible solver
    // std::string Alphat = "alphat";
};

//
class ConjugateHeatTransfer
{

private:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // All the parameter names
    const CHTParameterNames parameterName_;

public:

    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh, const CHTParameterNames names);

public:

    // 
    static std::optional<std::shared_ptr<ConjugateHeatTransfer>> CreateInstance
    (
        const Foam::fvMesh& mesh, const IOdictionary& adapterConfig
    );

public:

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);
};

}
}

#endif
