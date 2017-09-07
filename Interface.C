#include "Interface.H"

using namespace Foam;

preciceAdapter::Interface::Interface
(
    precice::SolverInterface & precice,
    const fvMesh& mesh,
    std::string meshName,
    std::vector<std::string> patchNames
)
:
precice_(precice),
meshName_(meshName),
patchNames_(patchNames)
{
    meshID_ = precice_.getMeshID(meshName_);

    for (uint i = 0; i < patchNames.size(); i++)
    {
        int patchID = mesh.boundaryMesh().findPatchID(patchNames.at(i));

        if (patchID == -1)
        {
            FatalErrorInFunction
                 << "ERROR: Patch '"
                 << patchNames.at(i)
                 << "' does not exist."
                 << exit(FatalError);
        }

        patchIDs_.push_back(patchID);
    }

    configureMesh_(mesh);

    //- An interface has only one data buffer, which is shared between several
    //  CouplingDataReaders and CouplingDataWriters.
    //  The initial allocation assumes scalar data.
    //  If CouplingDataReaders or -Writers have vector data, it is resized (TODO)
    dataBuffer_ = new double[numDataLocations_]();
}

void preciceAdapter::Interface::configureMesh_(const fvMesh& mesh)
{
    for (uint k = 0; k < patchIDs_.size(); k++)
    {
        numDataLocations_ +=
            mesh.boundaryMesh()[patchIDs_.at(k)].faceCentres().size();
    }

    int vertexIndex = 0;
    double vertices[3 * numDataLocations_];
    vertexIDs_ = new int[numDataLocations_];

    for (uint k = 0; k < patchIDs_.size(); k++)
    {
        const vectorField & faceCenters =
            mesh.boundaryMesh()[patchIDs_.at(k)].faceCentres();

        for (int i = 0; i < faceCenters.size(); i++)
        {
            vertices[vertexIndex++] = faceCenters[i].x();
            vertices[vertexIndex++] = faceCenters[i].y();
            vertices[vertexIndex++] = faceCenters[i].z();
        }
    }

    precice_.setMeshVertices(meshID_, numDataLocations_, vertices, vertexIDs_);
}


void preciceAdapter::Interface::addCouplingDataWriter
(
    std::string dataName,
    CouplingDataUser * couplingDataWriter
)
{
    couplingDataWriter->setDataID(precice_.getDataID(dataName, meshID_));
    couplingDataWriter->setPatchIDs(patchIDs_);
    couplingDataWriters_.push_back(couplingDataWriter);

    if (couplingDataWriter->hasVectorData())
    {
        // TODO: Resize buffer for vector data (if not already resized)
    }
}


void preciceAdapter::Interface::addCouplingDataReader
(
    std::string dataName,
    preciceAdapter::CouplingDataUser * couplingDataReader
)
{
    couplingDataReader->setDataID(precice_.getDataID(dataName, meshID_));
    couplingDataReader->setPatchIDs(patchIDs_);
    couplingDataReaders_.push_back(couplingDataReader);

    if (couplingDataReader->hasVectorData())
    {
        // TODO: Resize buffer for vector data (if not already resized)
    }
}


void preciceAdapter::Interface::readCouplingData()
{
    if (precice_.isReadDataAvailable())
    {
        for (uint i = 0; i < couplingDataReaders_.size(); i++)
        {
            preciceAdapter::CouplingDataUser *
                couplingDataReader = couplingDataReaders_.at(i);

            if (couplingDataReader->hasVectorData())
            {
                precice_.readBlockVectorData
                (
                    couplingDataReader->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_
                );
            }
            else
            {
                precice_.readBlockScalarData
                (
                    couplingDataReader->dataID(),
                    numDataLocations_,
                    vertexIDs_,
                    dataBuffer_
                );
            }

            couplingDataReader->read(dataBuffer_);
        }
    } // TODO: Else what?
}

void preciceAdapter::Interface::writeCouplingData()
{
    for (uint i = 0; i < couplingDataWriters_.size(); i++)
    {
        preciceAdapter::CouplingDataUser *
            couplingDataWriter = couplingDataWriters_.at(i);

        couplingDataWriter->write(dataBuffer_);

        if (couplingDataWriter->hasVectorData())
        {
            precice_.writeBlockVectorData
            (
                couplingDataWriter->dataID(),
                numDataLocations_,
                vertexIDs_,
                dataBuffer_
            );
        }
        else
        {
            precice_.writeBlockScalarData
            (
                couplingDataWriter->dataID(),
                numDataLocations_,
                vertexIDs_,
                dataBuffer_
            );
        }
    }
}

preciceAdapter::Interface::~Interface()
{
    for (uint i = 0; i < couplingDataReaders_.size(); i++)
    {
        delete couplingDataReaders_.at(i);
    }
    couplingDataReaders_.clear();

    for (uint i = 0; i < couplingDataWriters_.size(); i++)
    {
        delete couplingDataWriters_.at(i);
    }
    couplingDataWriters_.clear();

    delete [] vertexIDs_;

    delete [] dataBuffer_;

}
