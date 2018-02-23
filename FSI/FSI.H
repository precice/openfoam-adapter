#ifndef FSI_H
#define FSI_H

#include "Interface.H"

/* TODO: Include here the headers of the respective coupling data users.
/  This should look like:
    #include "FSI/Displacements.H"
    #include "FSI/Forces.H"
*/

#include "fvCFD.H"

// YAML reader - Used to read the adapter's configuration file.
#include "yaml-cpp/yaml.h"

namespace preciceAdapter
{
namespace FSI
{

class FluidStructureInteraction
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    /* TODO: Declare here any parameters that should be read from
    /  the configuration file. See CHT/CHT.H for reference.
    /  We want to support in-house solvers with different field names,
    /  so make sure to declare these here.
    */

protected:

    //- Read the FSI-related options from the adapter's configuraiton file
    bool readConfig(const YAML::Node adapterConfig);

public:

    //- Constructor
    FluidStructureInteraction(const Foam::fvMesh& mesh);

    //- Configure
    bool configure(const YAML::Node adapterConfig);

    //- Add coupling data writers
    void addWriters(std::string dataName, Interface * interface);

    //- Add coupling data readers
    void addReaders(std::string dataName, Interface * interface);
};

}
}

#endif
