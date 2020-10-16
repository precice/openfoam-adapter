#ifndef COUPLINGDATAUSER_H
#define COUPLINGDATAUSER_H

#include <vector>

namespace preciceAdapter
{

class CouplingDataUser
{

public:

    enum DataType
    {
        DT_Scalar = 0,
        DT_Vector = 1
    };

protected:

    //TODO: move protected class memembers to private and add getter methods, Thomas Enzinger

    //- Type of the coupling data (scalar or vector)
    const DataType dataType_ ;

    //- OpenFOAM patches that form the interface
    std::vector<int> patchIDs_;

    //- preCICE data ID
    int dataID_;

private:

    CouplingDataUser() = default;

public:

    //- Constructor
    CouplingDataUser(DataType type);

    //- Returns true if the data are scalar
    bool hasScalarData();

    //- Returns true if the data are vector
    bool hasVectorData();

    //- Set the preCICE data ID
    void setDataID(int dataID);

    //- Get the preCICE data ID
    int dataID();

    //- Set the patch IDs that form the interface
    void setPatchIDs(std::vector<int> patchIDs);

    //- Write the coupling data to the buffer
    virtual void write(std::vector<double> &dataBuffer, bool meshConnectivity, const unsigned int dim) = 0;

    //- Read the coupling data from the buffer
    virtual void read(const std::vector<double> &dataBuffer, const unsigned int dim) = 0;

    //- Destructor
    virtual ~CouplingDataUser() {}

};

}

#endif
