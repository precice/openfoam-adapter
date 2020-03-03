//
// Created by keefe on 3/2/20.
//
#include "SolverInterfaceMock.hpp"

// NOLINTNEXTLINE: Static string required for preCICE actions
const std::string precice::SolverInterface::readIterationCheckpoint = "readIterationCheckpoint";
// NOLINTNEXTLINE: Static string required for preCICE actions
const std::string precice::SolverInterface::writeIterationCheckpoint = "writeIterationCheckpoint";
// NOLINTNEXTLINE: Static string required for preCICE actions
const std::string precice::SolverInterface::initialDataName = "initialData";

// @brief Name of action for reading iteration checkpoint.
const std::string& precice::constants::actionReadIterationCheckpoint(){
    return precice::SolverInterface::readIterationCheckpoint;
};

// @brief Name of action for writing iteration checkpoint
const std::string& precice::constants::actionWriteIterationCheckpoint(){
    return precice::SolverInterface::writeIterationCheckpoint;
};

const std::string& precice::constants::actionWriteInitialData(){
    return precice::SolverInterface::writeIterationCheckpoint;
};

void integrationtest::writeCouplingTest(Mock<precice::SolverInterface>* mock){
    // int mesh_values = 0;
    // int data_values = 0;
    // When(Method(*mock,getMeshID)).AlwaysReturn(++mesh_values);
    // When(Method(*mock,getDimensions)).AlwaysReturn(3);
    // When(Method(*mock,getDataID)).AlwaysReturn(++data_values);
    
    // When(Method(*mock,getMeshID)).AlwaysReturn(++mesh_values);

    
}

void integrationtest::configureTest(Mock<precice::SolverInterface>* mock){

}