#ifndef FF_Height_SWE_H
#define FF_Height_SWE_H

#include "CouplingDataUser.H"

#include "fvCFD.H"

namespace preciceAdapter
{
namespace FF
{

//- Class that writes and reads height_SWE
class height_SWE : public CouplingDataUser
{

private:

    //- height_SWE field
    Foam::volScalarField * alpha_;
    Foam::volScalarField * p_rgh_;
    // Foam::volScalarField * p_;
    const Foam::fvMesh* mesh_;


public:

    //- Constructor
    height_SWE
    (
        const Foam::fvMesh& mesh,
        const std::string nameA_,
        const std::string namePrgh_
        // const std::string nameP_

    );

    //- Write the height_SWE values into the buffer
    void write(double * buffer, bool meshConnectivity, const unsigned int dim);

    //- Read the height_SWE values from the buffer
    void read(double * buffer, const unsigned int dim);

    //- Get cell size on the boundary
    double getBoundaryCellSize();

};

}
}

// NOTE: In order to couple another variable, you need to add another
// subclass of the CouplingDataUser. Take this file and Pressure.C
// as an example and look for more notes in the Adapter.C and Adapter.H.
// Essentially, the new class needs to implement a constructor (which also
// sets the dataType_) and implement the write() and read() methods.
// Then, you need to create objects of this class in the Adapter.C,
// whenever requested from the configuration file (see notes).

#endif
