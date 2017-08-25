#include "Interface.H"

using namespace Foam;

preciceAdapter::Interface::Interface( precice::SolverInterface & precice, const fvMesh& mesh, std::string meshName, std::vector<std::string> patchNames ) :
	_precice( precice ),
	_meshName( meshName ),
	_patchNames( patchNames ),
	_numDataLocations( 0 ),
	_numDims( 3 )
{
	Info << "---[preciceAdapter]   interface constructor begin" << nl;
	_meshID = _precice.getMeshID( _meshName );
	Info << "---[preciceAdapter]   interface constructor getMeshID complete" << nl;

	for( uint i = 0 ; i < patchNames.size() ; i++ )
	{
		int patchID = mesh.boundaryMesh().findPatchID( patchNames.at( i ) );
		if( patchID == -1 )
		{
			// BOOST_LOG_TRIVIAL( error ) << "ERROR: Patch '" << patchNames.at( i ) << "' does not exist.";
			exit( 1 );
		}
		Info << "---[preciceAdapter]   interface found patch" << nl;

		_patchIDs.push_back( patchID );
	}
	_configureMesh( mesh );
	Info << "---[preciceAdapter]   interface constructor configured mesh" << nl;

	/* An interface has only one data buffer, which is shared between several CouplingDataReaders and CouplingDataWriters
	   The initial allocation assumes scalar data, if CouplingDataReaders or -Writers have vector data, it is resized (TODO) */
	_dataBuffer = new double[_numDataLocations]();
	Info << "---[preciceAdapter]   interface constructor end" << nl;
}

void preciceAdapter::Interface::_configureMesh( const fvMesh& mesh )
{
	Info << "---[preciceAdapter]   interface configure mesh begin" << nl;
	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		_numDataLocations += mesh.boundaryMesh()[_patchIDs.at( k )].faceCentres().size();
	}
	int vertexIndex = 0;
	double vertices[3 * _numDataLocations];
	_vertexIDs = new int[_numDataLocations];

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		const vectorField & faceCenters = mesh.boundaryMesh()[_patchIDs.at( k )].faceCentres();

		for( int i = 0 ; i < faceCenters.size() ; i++ )
		{
			vertices[vertexIndex++] = faceCenters[i].x();
			vertices[vertexIndex++] = faceCenters[i].y();
			vertices[vertexIndex++] = faceCenters[i].z();
		}
	}
	_precice.setMeshVertices( _meshID, _numDataLocations, vertices, _vertexIDs );
	Info << "---[preciceAdapter]   interface configure mesh end" << nl;

}


void preciceAdapter::Interface::addCouplingDataWriter( std::string dataName, CouplingDataWriter * couplingDataWriter )
{
	couplingDataWriter->setDataID( _precice.getDataID( dataName, _meshID ) );
	couplingDataWriter->setPatchIDs( _patchIDs );
	_couplingDataWriters.push_back( couplingDataWriter );

	if( couplingDataWriter->hasVectorData() )
	{
		// TODO: Resize buffer for vector data (if not already resized)
	}
}


void preciceAdapter::Interface::addCouplingDataReader( std::string dataName, preciceAdapter::CouplingDataReader * couplingDataReader )
{
	couplingDataReader->setDataID( _precice.getDataID( dataName, _meshID ) );
	couplingDataReader->setPatchIDs( _patchIDs );
	_couplingDataReaders.push_back( couplingDataReader );

	if( couplingDataReader->hasVectorData() )
	{
		// TODO: Resize buffer for vector data (if not already resized)
	}
}


void preciceAdapter::Interface::readCouplingData()
{
	if( _precice.isReadDataAvailable() )
	{
		for( uint i = 0 ; i < _couplingDataReaders.size() ; i++ )
		{
			preciceAdapter::CouplingDataReader * couplingDataReader = _couplingDataReaders.at( i );

			if( couplingDataReader->hasVectorData() )
			{
				_precice.readBlockVectorData( couplingDataReader->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
			}
			else
			{
				_precice.readBlockScalarData( couplingDataReader->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
			}
			couplingDataReader->read( _dataBuffer );
		}
	}
}

void preciceAdapter::Interface::writeCouplingData()
{
	for( uint i = 0 ; i < _couplingDataWriters.size() ; i++ )
	{
		preciceAdapter::CouplingDataWriter * couplingDataWriter = _couplingDataWriters.at( i );
		couplingDataWriter->write( _dataBuffer );

		if( couplingDataWriter->hasVectorData() )
		{
			_precice.writeBlockVectorData( couplingDataWriter->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
		}
		else
		{
			_precice.writeBlockScalarData( couplingDataWriter->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
		}
	}
}

preciceAdapter::Interface::~Interface()
{

	// BOOST_LOG_TRIVIAL( info ) << "Destroying interface...";

	for ( uint i = 0 ; i < _couplingDataReaders.size() ; i++ )
	{
		delete _couplingDataReaders.at( i );
	}
	_couplingDataReaders.clear();

	for ( uint i = 0 ; i < _couplingDataWriters.size() ; i++ )
	{
		delete _couplingDataWriters.at( i );
	}
	_couplingDataWriters.clear();

	delete [] _vertexIDs;
	delete [] _dataBuffer;

}
