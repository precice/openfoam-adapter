#include "TestAdapter.H"
#include "Interface.H"
#include "Utilities.H"

#include "MockpreCICE/SolverInterfaceMock.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <iomanip>
#include <cmath>


using namespace Foam;
using namespace fakeit;
preciceAdapter::TestAdapter::TestAdapter(const Time& runTime, const fvMesh& mesh)
    :Adapter(runTime, mesh)
{
    adapterInfo("Tests starting.", "info");
    precice_ = &mock.get();
    return;
}

void preciceAdapter::TestAdapter::testConfigure()
{
    adapterInfo("Configuring.", "info");
    mock.Reset();

    // Setup mock to return correct mesh ids for Temperature and Heat-Flux
    Fake(Method(mock,getMeshID)); 
    When(Method(mock, getMeshID)).AlwaysDo([this](std::string a)->int{return mesh_id[a];});
    
    Fake(Method(mock,getDataID)); 
    When(Method(mock, getDataID)).AlwaysDo([this](std::string a, int b)->int{return data_id[a];});
    
    // Dimensions for this simulation is 3
    When(Method(mock, getDimensions)).AlwaysReturn(3);
    
    // Match dt to that set in controlDict
    When(Method(mock, initialize)).AlwaysReturn(0.01);
    
    // Not required to write initial data for tutorial
    When(Method(mock, isActionRequired).Using(precice::constants::actionWriteInitialData())).AlwaysReturn(false);
    When(Method(mock, isActionRequired).Using(precice::constants::actionReadIterationCheckpoint())).AlwaysReturn(true);
    When(Method(mock, isActionRequired).Using(precice::constants::actionWriteIterationCheckpoint())).AlwaysReturn(true);

    // Creating stub when action is marked as fulfilled
    Fake(Method(mock, markActionFulfilled));
    // Not required to read initail data for tutorial
    When(Method(mock, isReadDataAvailable)).Return(false);

    // Getting and setting mesh vertices not necessary for tutorial. Set to stub.
    Fake(Method(mock, setMeshVertices));
    Fake(Method(mock, getMeshVertexIDsFromPositions));
    Fake(Method(mock, setMeshTriangleWithEdges));

    // Initialize data not necessary for this tutorial. Set to stub.
    Fake(Method(mock, initializeData));

    adapterInfo("Configuring.", "info");

    configure();

    return;
}

void preciceAdapter::TestAdapter::testExecute()
{   
    // Running execute
    adapterInfo("Executing.", "info");
    // Restting mock to prevent any strange side-effects
    mock.Reset();

    // Open file that contains data that is read from preCICE by OpenFOAM
    std::string readFileName = "./Dict/HeatFlux" + std::to_string(coupling_step) + "_" + std::to_string(iteration_step);
    ifstream readFile;   
    readFile.open(readFileName);
    // Create buffer to store data read from preCICE by OpenFOAM
    double readData[read_data_size];
    // Store data read from preCICE by OpenFOAM in buffer
    for(int i=0; i<read_data_size; i++){
        readFile >> readData[i];
    }

    // Open file that contains data that is written to preCICE by OpenFOAM
    std::string writeFileName = "./Dict/Temperature" + std::to_string(coupling_step) + "_" + std::to_string(iteration_step);
    ifstream writeFile;   
    writeFile.open(writeFileName);
    // Create buffer to store data read from preCICE by OpenFOAM
    double writeData[write_data_size];
    double writeDataError[write_data_size];
    // Store data read from preCICE by OpenFOAM in buffer
    for(int i=0; i<write_data_size; i++){
        writeFile >> writeData[i];
    }

    When(Method(mock, readBlockScalarData)).Do([this, &readData](
                            int dataID,
                            int numDataLocations_,
                            const int* vertexIDs_,
                            double* dataBuffer_
                            ){
                                for(int i=0; i < numDataLocations_; i++){
                                    dataBuffer_[i] = readData[i];
                                }
                            });

    When(Method(mock, writeBlockScalarData)).Do([this, &writeData, &writeDataError](
                            int dataID,
                            int numDataLocations_,
                            const int* vertexIDs_,
                            double* dataBuffer_
                            ){
                                for(int i=0; i < numDataLocations_; i++){
                                    writeDataError[i] = writeData[i] - dataBuffer_[i];
                                }
                            });

    bool isReadIterationCheckpoint;
    bool isWriteIterationCheckpoint;
    bool isCouplingOngoing;
    bool isTimeWindowComplete;
    if(iteration_step == implicit_inner_steps[coupling_step]){
        isReadIterationCheckpoint = false;
        isWriteIterationCheckpoint = true;
        isTimeWindowComplete = true;
    } 
    else{
        isReadIterationCheckpoint = true;
        isWriteIterationCheckpoint = false;
        isTimeWindowComplete = false;
    }
    if (coupling_step == 6){
        isCouplingOngoing = false;
    }
    else{
        isCouplingOngoing = true;
    }
    When(Method(mock, isActionRequired).Using(precice::constants::actionReadIterationCheckpoint())).AlwaysReturn(isReadIterationCheckpoint);
    When(Method(mock, isActionRequired).Using(precice::constants::actionWriteIterationCheckpoint())).AlwaysReturn(isWriteIterationCheckpoint);
    When(Method(mock, isCouplingOngoing)).AlwaysReturn(isCouplingOngoing);
    When(Method(mock, isReadDataAvailable)).AlwaysReturn(true);
    When(Method(mock, isTimeWindowComplete)).AlwaysReturn(isTimeWindowComplete);
    
    // Advance always returns dt value of 0.01
    When(Method(mock, advance)).AlwaysReturn(0.01);
    // Creating stub when action is marked as fulfilled
    Fake(Method(mock, markActionFulfilled));
    Fake(Method(mock, finalize));

    // Run execute
    execute();
    if (coupling_step < 6){
        for (int i=0; i < write_data_size; i++){
            if (std::abs(writeDataError[i]) > 1e-5){
                adapterInfo("Temperature Data at index " + std::to_string(i) + " not matching!", "error-deferred");
                adapterInfo("Reference values is: " + std::to_string(writeData[i]) 
                    + ", Buffer value is: " + std::to_string(writeData[i] - writeDataError[i]), "error-deferred");
		testsPassed = false;
            }
            
        }
    }

    // Update iteration steps and coupling steps

    if(iteration_step == implicit_inner_steps[coupling_step]){
        coupling_step++;
        iteration_step = 1;
    }
    else{
        iteration_step++;
    }

    readFile.close();
    writeFile.close();

    return;
}


void preciceAdapter::TestAdapter::testAdjustSolverTimeStep()
{
    adapterInfo("Adjusting Timestep", "info");
    adjustSolverTimeStep();
    return;
}


void preciceAdapter::TestAdapter::testEnd()
{
    if(testsPassed){
        adapterInfo("All integration tests passed!", "info");
    }
    else{
        adapterInfo("Tests failed! See errors for more details.", "error-deferred");
    }
    mock.Reset();
    When(Method(mock, isCouplingOngoing)).AlwaysReturn(false);
    end();
    return;
}


preciceAdapter::TestAdapter::~TestAdapter()
{
}

