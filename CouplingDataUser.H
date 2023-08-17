#ifndef COUPLINGDATAUSER_H
#define COUPLINGDATAUSER_H

#include "Utilities.H"
#include <vector>
#include <string>

namespace preciceAdapter
{
// A small enum to deal with the different locations of the
// coupling interface
enum class LocationType
{
    none,
    faceCenters,
    faceNodes,
    volumeCenters
};

class CouplingDataUser
{

protected:
    enum DataType
    {
        scalar,
        vector
    };

    //- Type of the coupling data (scalar or vector)
    DataType dataType_ = scalar;

    //- OpenFOAM patches that form the interface
    std::vector<int> patchIDs_;

    //- Names of the OpenFOAM cell sets to be coupled (for volume coupling)
    std::vector<std::string> cellSetNames_;

    //- data name
    std::string dataName_;

    //- location type of the interface
    LocationType locationType_ = LocationType::none;

public:
    //- Constructor
    CouplingDataUser();

    //- Returns true if the data are scalar
    bool hasScalarData();

    //- Returns true if the data are vector
    bool hasVectorData();

    //- Set the data name
    void setDataName(std::string dataName);

    //- Get the data name
    const std::string& dataName();

    //- Set the patch IDs that form the interface
    void setPatchIDs(std::vector<int> patchIDs);

    //- Set the cellSetNames that form the overlapping cells of the interface
    void setCellSetNames(std::vector<std::string> cellSetNames);

    //- Set the locations type of the interface
    void setLocationsType(LocationType locationsType);

    // Check if the dataset supports this interface nodes location
    void checkDataLocation(const bool meshConnectivity) const;

    //- option to initialize data in derived data classes
    virtual void initialize();

    //- Write the coupling data to the buffer
    // Returns the number of entries that were filles in the buffer (nComp * vertices)
    virtual std::size_t write(double* dataBuffer, bool meshConnectivity, const unsigned int dim) = 0;

    //- Read the coupling data from the buffer
    virtual void read(double* dataBuffer, const unsigned int dim) = 0;

    //- Given the meshConnectivity, return if the underlying loactionType of the
    //- interface nodes is supported by the data set
    virtual bool isLocationTypeSupported(const bool meshConnectivity) const = 0;

    //- Get the name of the current data field
    virtual std::string getDataName() const = 0;

    //- Destructor
    virtual ~CouplingDataUser() {}
};

}

#endif
