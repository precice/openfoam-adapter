#ifndef GENERIC_VOLVECTORFIELD_H
#define GENERIC_VOLVECTORFIELD_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace Volume_Coupling
{

//- Class that writes and reads Generic volVector field
class Generic_volVectorField : public CouplingDataUser
{

private:

    //- Generic field
    Foam::volVectorField * generic_volVectorField_ ;

public:

  //- Constructor
  Generic_volVectorField
  (
      const Foam::fvMesh& mesh,
      const std::string nameGeneric_volVectorField
  );

  //- Write the volVectorField values into the buffer
  void write(double * buffer, bool meshConnectivity, const unsigned int dim);

  //- Read the volVectorField values from the buffer
  void read(double * buffer, const unsigned int dim);

  //- Destructor
  virtual ~Generic_volVectorField() {}
};

}
}

#endif
