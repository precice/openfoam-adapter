// TODO: From Lucia, just for development

#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>
#include <vector>
#include <boost/log/trivial.hpp>
#include "fvCFD.H"
#include "CouplingDataUser/CouplingDataReader/CouplingDataReader.H"
#include "CouplingDataUser/CouplingDataWriter/CouplingDataWriter.H"
#include "precice/SolverInterface.hpp"


namespace preciceAdapter
{

class Interface
{
protected:

	/**
	 * @brief preCICE's solver interface object
	 */
	precice::SolverInterface & _precice;

	/**
	 * @brief Mesh name used in the preCICE configuration
	 */
	std::string _meshName;

	/**
	 * @brief Mesh ID assigned by preCICE to the interface
	 */
	int _meshID;

	/**
	 * @brief Vector of patch names that make up the interface
	 */
	std::vector<std::string> _patchNames;

	/**
	 * @brief Vector of patch IDs that make up the interface
	 */
	std::vector<int> _patchIDs;

	/**
	 * @brief Number of vertices of the interface
	 */
	int _numDataLocations;

	/**
	 * @brief Vertex IDs assigned by preCICE
	 */
	int * _vertexIDs;

	/**
	 * @brief Number of dimensions (fixed to 3, cannot handle 2)
	 */
	int _numDims;

	/**
	 * @brief Buffer for the coupling data
	 */
	double * _dataBuffer;

	/**
	 * @brief Vector of CouplingDataReaders
	 */
     // TODO
	std::vector<CouplingDataReader*> _couplingDataReaders;

	/**
	 * @brief Vector of CouplingDataWriters
	 */
     // TODO
	std::vector<CouplingDataWriter*> _couplingDataWriters;

	/**
	 * @brief Extracts locations of face centers and exposes them to preCICE with setMeshVertices
	 * TODO: Create a mesh of nodes instead of face centers?
	 */
	void _configureMesh( const Foam::fvMesh& mesh );

public:

	/**
	 * @brief Interface constructor
	 * @param precice: preCICE solver interface object
	 * @param mesh: OpenFOAM's mesh
	 * @param meshName: Mesh name assigned to the interface in the preCICE configuration
	 * @param patchNames: Vector of patch names that make up the interface
	 */
	Interface( precice::SolverInterface & precice,
			   const Foam::fvMesh& mesh,
			   std::string meshName,
			   std::vector<std::string> patchNames
			   );

	/**
	 * @brief Adds a CouplingDataReader to the interface
	 */
     //TODO
	void addCouplingDataReader( std::string dataName, CouplingDataReader * couplingDataReader );

	/**
	 * @brief Adds a CouplingDataWriter to the interface
	 */
     //TODO
	void addCouplingDataWriter( std::string dataName, CouplingDataWriter * couplingDataWriter );

	/**
	 * @brief Calls read() on each couplingDataReader to read the coupling data from the buffer
	 * and apply the boundary conditions
	 */
	void readCouplingData();

	/**
	 * @brief Calls write() on each couplingDataWriter to extract the boundary data and write it
	 * into the buffer
	 */
	void writeCouplingData();

	/**
	 * @brief Destructor
	 */
	~Interface();

};

}

#endif // INTERFACE_H
