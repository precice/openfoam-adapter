#ifndef Momentum_H
#define Momentum_H

#include "Interface.H"

#include "Volume_Coupling/Generic_volScalarField.H"
#include "Volume_Coupling/Generic_volVectorField.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace Momentum
{

class Momentum
{

protected:

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    //- OpenFOAM Time object (we need to access the timestep to compute the pointMotionU)
    const Foam::Time& runTime_;

    
    //- Name of the Porosity
    std::string nameYour_Volume_Field_ = "Your_Volume_Field";

    /* TODO: Declare here any parameters that should be read from
    /  the configuration file. See CHT/CHT.H for reference.
    /  We want to support in-house solvers with different field names,
    /  so make sure to declare these here.
    */

protected:

    //- Read the Newcase-related options from the adapter's configuration file
    bool readConfig(const IOdictionary& adapterConfig);

public:

    //- Constructor
    //  We need also the runTime to access the timeName and the deltaT
    Momentum(const Foam::fvMesh& mesh, const Foam::Time& runTime);

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
