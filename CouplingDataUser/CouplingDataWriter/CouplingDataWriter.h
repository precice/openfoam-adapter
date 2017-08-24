#ifndef COUPLINGDATAWRITER_H
#define COUPLINGDATAWRITER_H

#include "../CouplingDataUser.h"

namespace preciceAdapter
{

class CouplingDataWriter : public CouplingDataUser
{

protected:

public:

	virtual void write( double * dataBuffer ) = 0;
	virtual ~CouplingDataWriter()
	{
	}

};

}

#endif // COUPLINGDATAWRITER_H
