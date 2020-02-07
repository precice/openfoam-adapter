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

void precice::SolverInterface::initializeData(){
    ON_CALL(*this, initializeDataMock());
}

bool precice::SolverInterface::isActionRequired(const std::string& actionName) const{
    EXPECT_CALL(*this, isActionRequiredMock(testing::_)).WillRepeatedly(testing::Return(true));
    return isActionRequiredMock(actionName);
};

const std::string& precice::constants::actionWriteInitialData(){
    mockConstants mock;
    EXPECT_CALL(mock, initialData()).WillOnce(testing::ReturnRef(precice::SolverInterface::initialDataName));
    return mock.initialData();
};

// @brief Name of action for writing iteration checkpoint
const std::string& precice::constants::actionWriteIterationCheckpoint(){
    mockConstants mock;
    EXPECT_CALL(mock, writeIterationCheckpoint()).WillRepeatedly(testing::ReturnRef(precice::SolverInterface::writeIterationCheckpoint));
    return mock.writeIterationCheckpoint();
};

// @brief Name of action for reading iteration checkpoint.
const std::string& precice::constants::actionReadIterationCheckpoint(){
    mockConstants mock;
    EXPECT_CALL(mock, readIterationCheckpoint()).WillRepeatedly(testing::ReturnRef(precice::SolverInterface::readIterationCheckpoint));
    return mock.readIterationCheckpoint();
};

double precice::SolverInterface::initialize(){
    EXPECT_CALL(*this, initializeMock()).WillOnce(testing::Return(Foam::TEST_TIME_STEP));
    return initializeMock();
}

void precice::SolverInterface::fulfilledAction(const std::string &actionName) const {
    EXPECT_CALL(*this, fulfilledActionMock(actionName));
    fulfilledActionMock(actionName);
}

double precice::SolverInterface::advance(double timestep){
    EXPECT_CALL(*this, advanceMock(0));
    return advanceMock(timestep);
}

bool precice::SolverInterface::isTimestepComplete() const{
    EXPECT_CALL(*this, isTimestepCompleteMock()).WillOnce(testing::Return(true));
    return isTimestepCompleteMock();
};

bool precice::SolverInterface::isCouplingOngoing(){
    EXPECT_CALL(*this, isCouplingOngoingMock()).WillOnce(testing::Return(false));
    return isCouplingOngoingMock();
};