#ifndef COUPLINGDATAREADER_H
#define COUPLINGDATAREADER_H

#include "../CouplingDataUser.h"

namespace preciceAdapter
{

class CouplingDataReader : public CouplingDataUser
{

protected:

public:

	virtual void read( double * dataBuffer ) = 0;
	virtual ~CouplingDataReader()
	{
	}

};

}

#endif // COUPLINGDATAREADER_H
