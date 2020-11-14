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
