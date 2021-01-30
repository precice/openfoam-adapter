#ifndef CHT_H
#define CHT_H

// forward declaration of foam space
namespace Foam
{

class word;
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
    Foam::word T = "T";
};

class ConjugateHeatTransfer
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // All the parameter names
    CHTParameterNames parameterName_;

protected:

    //- Read the CHT-related options from the adapter's configuraiton file
    bool readConfig(const IOdictionary& adapterConfig);

public:

    //- Constructor
    ConjugateHeatTransfer(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const IOdictionary& adapterConfig);

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);
};

}
}

#endif
