#ifndef MOMENTUM_YOUR_VOLUME_FIELD_H
#define MOMENTUM_YOUR_VOLUME_FIELD_H

#include "CouplingDataUser.H"
#include "fvCFD.H"

namespace preciceAdapter
{
namespace Momentum
{

//- Class that writes and reads Your Volume Field.
class Your_Volume_Field : public CouplingDataUser
{

private:
    
    //- Porosity field
    Foam::volScalarField * Your_Volume_Field_ ;

public:

    //- Constructor
    Your_Volume_Field
    (   
         const Foam::fvMesh& mesh,
         const std::string nameYour_Volume_Field
    );

    //- Write the porosity values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the porosity values from the buffer
    void read(double * buffer, const unsigned int dim);

};

}
}

#endif
