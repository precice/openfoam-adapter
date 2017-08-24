#ifndef COUPLINGDATAUSER_H
#define COUPLINGDATAUSER_H

#include <vector>
#include <string>

namespace preciceAdapter
{

class CouplingDataUser
{

protected:

    enum DataType {vector, scalar};

    /**
     * @brief Indicates whether the data is scalar or vector
     */
	DataType _dataType;

    /**
     * @brief Size of the buffer (if it is vector data, the real size in memory is _bufferSize*dims)
     */
	int _bufferSize;

    /**
     * @brief Vector of patch IDs that make up the interface
     */
	std::vector<int> _patchIDs;

    /**
     * @brief preCICE data ID
     */
	int _dataID;

public:

	CouplingDataUser();

    /**
     * @brief Returns true if the data is vector
     */
	bool hasVectorData();

    /**
     * @brief Returns true if the data is scalar
     */
	bool hasScalarData();

    /**
     * @brief Set the value of the buffer size
     */
	void setSize( int size );

    /**
     * @brief Set the preCICE data ID
     */
	void setDataID( int getDataID );

    /**
     * @brief Set the patch IDs that make up the interface
     */
	void setPatchIDs( std::vector<int> patchIDs );

    /**
     * @brief Returns the preCICE data ID
     */
	int getDataID();

};

}

#endif // COUPLINGDATAUSER_H
