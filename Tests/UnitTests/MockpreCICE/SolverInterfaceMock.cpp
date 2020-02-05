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

bool precice::SolverInterface::isActionRequired(const std::string& actionName) const{
    ON_CALL(*this, isActionRequiredMock(testing::_)).WillByDefault(testing::Return(true));
    return isActionRequiredMock(actionName);
};

const std::string& precice::constants::actionWriteInitialData(){
    mockConstants mock;
    std::cout << "Before ON_CALL" << std::endl;
    ON_CALL(mock, initialData()).WillByDefault(testing::ReturnRef(precice::SolverInterface::initialDataName));
    std::cout << "After ON_CALL" << std::endl;

    return mock.initialData();
};

// @brief Name of action for writing iteration checkpoint
const std::string& precice::constants::actionWriteIterationCheckpoint(){
    mockConstants mock;
    ON_CALL(mock, writeIterationCheckpoint()).WillByDefault(testing::ReturnRef(precice::SolverInterface::writeIterationCheckpoint));
    return mock.writeIterationCheckpoint();
};

// @brief Name of action for reading iteration checkpoint.
const std::string& precice::constants::actionReadIterationCheckpoint(){
    mockConstants mock;
    ON_CALL(mock, readIterationCheckpoint()).WillByDefault(testing::ReturnRef(precice::SolverInterface::readIterationCheckpoint));
    return mock.readIterationCheckpoint();
};

double precice::SolverInterface::initialize(){
    ON_CALL(*this, initializeMock()).WillByDefault(testing::Return(0.5));
    return initializeMock();
}