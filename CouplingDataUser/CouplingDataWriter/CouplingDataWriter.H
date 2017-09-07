#ifndef COUPLINGDATAWRITER_H
#define COUPLINGDATAWRITER_H

#include "../CouplingDataUser.H"

namespace preciceAdapter
{

class CouplingDataWriter : public CouplingDataUser
{

public:

    virtual void write(double * dataBuffer) = 0;

    virtual ~CouplingDataWriter()
    {}

};

}

#endif // COUPLINGDATAWRITER_H
