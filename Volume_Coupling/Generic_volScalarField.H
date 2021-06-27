#ifndef GENERIC_VOLSCALARFIELD_H
#define GENERIC_VOLSCALARFIELD_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace Volume_Coupling
{

//- Class that writes and reads Generic volScalar field
class Generic_volScalarField : public CouplingDataUser
{

private:

    //- Generic field
    Foam::volScalarField * generic_volScalarField_ ;

public:

  //- Constructor
  Generic_volScalarField
  (
      const Foam::fvMesh& mesh,
      const std::string nameGeneric_volScalarField
  );

  //- Write the volScalarField values into the buffer
  void write(double * buffer, bool meshConnectivity, const unsigned int dim);

  //- Read the volScalarField values from the buffer
  void read(double * buffer, const unsigned int dim);

  //- Destructor
  virtual ~Generic_volScalarField() {}
};

}
}
#endif
