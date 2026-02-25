#ifndef PRECICEADAPTER_H
#define PRECICEADAPTER_H

#include "Interface.H"

// Conjugate Heat Transfer module
#include "CHT/CHT.H"

// Fluid-Structure Interaction module
#include "FSI/FSI.H"

// Fluid-Fluid module
#include "FF/FF.H"

#include "module-generic/Generic.H"

// NOTE: If you want to couple a new variable, include your module's header here.
// You also need to include it in the Make/files file.
// In case you use additional OpenFOAM symbols, you may also need to specify
// the respective libraries in the Make/options.

// OpenFOAM header files
#include "Time.H"
#include "fvMesh.H"

// preCICE Solver Interface
#include <precice/precice.hpp>

#if PRECICE_VERSION_GREATER_EQUAL(3, 2, 0)
using PreciceError = ::precice::Error;
#else
using PreciceError = std::runtime_error;
#endif

namespace preciceAdapter
{

class Adapter
{

private:
    //- Structure of the configuration of each coupling interface.
    //  Every interface needs to know the coupling mesh, the OpenFOAM
    //  patches that consist the coupling surface and the kinds
    //  of data that are exchanged.
    struct InterfaceConfig
    {
        std::string meshName;
        std::string locationsType;
        bool meshConnectivity;
        std::vector<std::string> patchNames;
        std::vector<std::string> cellSetNames;
        std::vector<struct FieldConfig> writeData;
        std::vector<struct FieldConfig> readData;
    };

    //- Configuration interfaces
    std::vector<struct InterfaceConfig> interfacesConfig_;

#ifdef ADAPTER_ENABLE_TIMINGS
    //- Timers
    clockValue timeInConfigRead_;
    clockValue timeInPreciceConstruct_;
    clockValue timeInMeshSetup_;
    clockValue timeInInitialize_;
    clockValue timeInCheckpointingSetup_;
    clockValue timeInWrite_;
    clockValue timeInAdvance_;
    clockValue timeInRead_;
    clockValue timeInCheckpointingRead_;
    clockValue timeInCheckpointingWrite_;
    clockValue timeInWriteResults_;
    clockValue timeInFinalize_;
#endif

    //- OpenFOAM runTime object
    const Foam::Time& runTime_;

    //- OpenFOAM fvMesh object
    const Foam::fvMesh& mesh_;

    // Configuration parameters used in the Adapter

    //- preCICE configuration file name
    Foam::word preciceConfigFilename_;

    //- preCICE participant name
    Foam::word participantName_;

    //- Switch for checkpointing
    bool checkpointing_ = false;

    //- Moving mesh checkpointing
    bool meshCheckPointed_ = false;

    //- Type of timestep (fixed, adjustable)
    bool adjustableTimestep_;

    //- Should the (fixed) timestep be stored and used?
    bool useStoredTimestep_ = false;

    //- Switch to enable the ConjugateHeatTransfer module
    bool CHTenabled_ = false;

    //- Switch to enable the FluidStructureInteraction module
    bool FSIenabled_ = false;

    //- Switch to enable the FluidFluid module
    bool FFenabled_ = false;

    // NOTE: Add a switch for your new module here

    //- Switch to enable General module
    bool genericModuleEnabled_ = false;

    //- Interfaces
    std::vector<Interface*> interfaces_;

    //- preCICE solver interface
    precice::Participant* precice_ = nullptr;

    //- preCICE solver interface initialized
    bool preciceInitialized_ = false;

    //- Conjugate Heat Transfer module object
    CHT::ConjugateHeatTransfer* CHT_ = nullptr;

    //- Fluid-Structure Interaction module object
    FSI::FluidStructureInteraction* FSI_ = nullptr;

    //- Fluid-Fluid module object
    FF::FluidFluid* FF_ = nullptr;

    //- Generic module object
    Generic::GenericInterface* Generic_ = nullptr;

    // NOTE: Add here a pointer for your new module object

    // Timesteps

    //- Timestep used by the solver
    double timestepSolver_;

    //- Stored (fixed) timestep
    double timestepStored_;

    // Checkpointing

    //- Checkpointed time (value)
    Foam::scalar couplingIterationTimeValue_;

    //- Checkpointed time (index)
    Foam::label couplingIterationTimeIndex_;

    // Vectors of pointers to the checkpointed mesh fields and their copies

    //- Checkpointed mesh points, oldPoints
    Foam::pointField* meshPoints_ = nullptr;
    Foam::pointField* meshOldPoints_ = nullptr;

    //- Checkpointed surfaceScalarField mesh fields (phi)
    std::vector<Foam::surfaceScalarField*> meshSurfaceScalarFields_;

    //- Checkpointed surfaceScalarField mesh fields (copies)
    std::vector<Foam::surfaceScalarField*> meshSurfaceScalarFieldCopies_;

    // Vectors of pointers to the checkpointed fields and their copies

    //- Checkpointed volScalarField fields
    std::vector<Foam::volScalarField*> volScalarFields_;

    //- Checkpointed volScalarField fields (copies)
    std::vector<Foam::volScalarField*> volScalarFieldCopies_;

    //- Checkpointed volVectorField fields
    std::vector<Foam::volVectorField*> volVectorFields_;

    //- Checkpointed volVectorField fields (copies)
    std::vector<Foam::volVectorField*> volVectorFieldCopies_;

    //- Checkpointed surfaceScalarField fields
    std::vector<Foam::surfaceScalarField*> surfaceScalarFields_;

    //- Checkpointed surfaceScalarField fields (copies)
    std::vector<Foam::surfaceScalarField*> surfaceScalarFieldCopies_;

    //- Checkpointed surfaceVectorField fields
    std::vector<Foam::surfaceVectorField*> surfaceVectorFields_;

    //- Checkpointed surfaceVectorField fields (copies)
    std::vector<Foam::surfaceVectorField*> surfaceVectorFieldCopies_;

    //- Checkpointed pointScalarField fields
    std::vector<Foam::pointScalarField*> pointScalarFields_;

    //- Checkpointed pointScalarField fields (copies)
    std::vector<Foam::pointScalarField*> pointScalarFieldCopies_;

    //- Checkpointed pointVectorField fields
    std::vector<Foam::pointVectorField*> pointVectorFields_;

    //- Checkpointed pointVectorField fields (copies)
    std::vector<Foam::pointVectorField*> pointVectorFieldCopies_;

    //- Checkpointed volTensorField fields
    std::vector<Foam::volTensorField*> volTensorFields_;

    //- Checkpointed volTensorField fields (copies)
    std::vector<Foam::volTensorField*> volTensorFieldCopies_;

    //- Checkpointed surfaceTensorField fields
    std::vector<Foam::surfaceTensorField*> surfaceTensorFields_;

    //- Checkpointed surfaceTensorField fields (copies)
    std::vector<Foam::surfaceTensorField*> surfaceTensorFieldCopies_;

    //- Checkpointed pointTensorField fields
    std::vector<Foam::pointTensorField*> pointTensorFields_;

    //- Checkpointed pointTensorField fields (copies)
    std::vector<Foam::pointTensorField*> pointTensorFieldCopies_;

    //- Checkpointed volSymmTensorField fields
    std::vector<Foam::volSymmTensorField*> volSymmTensorFields_;

    //- Checkpointed volSymmTensorField fields (copies)
    std::vector<Foam::volSymmTensorField*> volSymmTensorFieldCopies_;


    // NOTE: Declare additional vectors for any other types required.

    // Configuration

    //- Read the adapter's configuration file
    void configFileRead();

    //- Helper to read a list of field configurations, which can contain either words, dictionaries, or both
    void readFieldConfigs(const std::string& listName, Foam::ITstream& stream, std::vector<FieldConfig>& configs);

    //- Check the adapter's configuration file
    bool configFileCheck(const std::string adapterConfigFileName);

    // Methods communicating with preCICE

    //- Initialize preCICE and exchange the first data
    void initialize();

    //- Finalize and destroy preCICE
    void finalize();

    //- Advance preCICE
    void advance();

    //- Read the coupling data at each interface
    void readCouplingData(double relativeReadTime);

    //- Write the coupling data at each interface
    void writeCouplingData();

    //- Adjust the timestep of the solver according to preCICE
    // and read data associated to the calculated time step length
    void adjustSolverTimeStepAndReadData();

    //- Determine if the coupling is still happening
    bool isCouplingOngoing();

    //- Determine if the coupling timestep has been completed
    bool isCouplingTimeWindowComplete();

    //- Determine if a checkpoint must be read
    bool requiresReadingCheckpoint();

    //- Determine if a checkpoint must be written
    bool requiresWritingCheckpoint();

    // Methods for checkpointing

    //- Configure the mesh checkpointing
    void setupMeshCheckpointing();

    //- Configure the checkpointing
    void setupCheckpointing();

    //- Remove checkpointed fields which are not used by OpenFOAM anymore
    void pruneCheckpointedFields();


    //- Make a copy of the runTime object
    void storeCheckpointTime();

    //- Restore the copy of the runTime object
    void reloadCheckpointTime();

    //- Store the locations of the mesh points
    void storeMeshPoints();

    //- Restore the locations of the mesh points
    void reloadMeshPoints();

    // Add mesh checkpoint fields, depending on the type

    //- Add a surfaceScalarField mesh field (phi)
    void addMeshCheckpointField(Foam::surfaceScalarField& field);

    //- Add a volScalarField to checkpoint
    void addCheckpointField(volScalarField* field);

    //- Add a volVectorField to checkpoint
    void addCheckpointField(volVectorField* field);

    //- Add a surfaceScalarField to checkpoint
    void addCheckpointField(surfaceScalarField* field);

    //- Add a surfaceVectorField to checkpoint
    void addCheckpointField(surfaceVectorField* field);

    //- Add a pointScalarField to checkpoint
    void addCheckpointField(pointScalarField* field);

    //- Add a pointVectorField to checkpoint
    void addCheckpointField(pointVectorField* field);

    // NOTE: Add here methods to add other object types to checkpoint,
    // if needed.

    //- Add a volTensorField to checkpoint
    void addCheckpointField(volTensorField* field);

    //- Add a surfaceTensorField to checkpoint
    void addCheckpointField(surfaceTensorField* field);

    //- Add a pointTensorField to checkpoint
    void addCheckpointField(pointTensorField* field);

    //- Add a volSymmTensorField to checkpoint
    void addCheckpointField(volSymmTensorField* field);

    //- Read the checkpoint - restore the mesh fields and time
    void readMeshCheckpoint();

    //- Read the checkpoint - restore the fields and time
    void readCheckpoint();

    //- Write the checkpoint - store the mesh fields and time
    void writeMeshCheckpoint();

    //- Write the checkpoint - store the fields and time
    void writeCheckpoint();

    //- Destroy the preCICE interface and delete the allocated
    //  memory in a proper way. Called by the destructor.
    void teardown();

public:
    // Methods called by the functionObject

    //- Constructor
    Adapter(const Foam::Time& runTime, const Foam::fvMesh& mesh);

    //- Setup the adapter's configuration
    void configure();

    //- Called by the functionObject's execute()
    void execute();

    //- Called by the functionObject's adjustTimeStep()
    void adjustTimeStep();

    //- Called by the functionObject's end()
    void end();

    //- Destructor
    ~Adapter();
};

}

#endif
