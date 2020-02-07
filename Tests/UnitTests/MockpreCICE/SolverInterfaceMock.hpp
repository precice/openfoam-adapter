//
// Created by keefe on 3/2/20.
//

#pragma once
#ifndef SolverInterface_H
#define SolverInterface_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Tests/UnitTests/MockFOAM/timeMock.h"

namespace precice{
    class SolverInterface{
    public:
        const static std::string readIterationCheckpoint;
        const static std::string writeIterationCheckpoint;
        const static std::string initialDataName;
        SolverInterface (
                const std::string& participantName,
                int                solverProcessIndex,
                int                solverProcessSize)
        {
//            solverConstructor();
        }

        MOCK_METHOD(void, solverConstructorMock, ());
        void solverConstructor(){
            EXPECT_CALL(*this, solverConstructorMock());
            solverConstructorMock();
        }

        MOCK_METHOD(void, configureMock, (std::string&));
        void configure(std::string & word){
            std::string testWord = "./precice-config.xml";
            EXPECT_CALL(*this, configureMock(testWord)).Times(1);
            configureMock(word);
        }

        double initialize();
        MOCK_METHOD(double, initializeMock, ());

        void initializeData();
        MOCK_METHOD(void, initializeDataMock, ());

        double advance(double timestep);
        MOCK_METHOD(double, advanceMock, (double));

        MOCK_METHOD(void, finalize, ());
        MOCK_METHOD(int, getDimensions, (), (const));
        // Auxiliary methods
        MOCK_METHOD(bool, isCouplingOngoingMock, ());
        bool isCouplingOngoing();
        MOCK_METHOD(bool, isReadDataAvailable, (), (const));
        MOCK_METHOD(bool, isWriteDataRequired, (double), (const));

        MOCK_METHOD(bool, isTimestepCompleteMock, (), (const));
        bool isTimestepComplete() const;

        bool isActionRequired(const std::string& actionName) const;
        MOCK_METHOD(bool, isActionRequiredMock, (const std::string&), (const));
        void fulfilledAction(const std::string& actionName) const;
        MOCK_METHOD(void, fulfilledActionMock, (const std::string&), (const));

        // Mesh Methods
        MOCK_METHOD(bool, hasMesh, (const std::string&), (const));
        MOCK_METHOD(std::set<int>, getMeshIDs, (), (const));

        MOCK_METHOD(int, setMeshVertex, (int , const double*));
        MOCK_METHOD(int, getMeshVertexSize, (int), (const));

        MOCK_METHOD(void, setMeshVertices, (int, int, const double*, int*));
        MOCK_METHOD(void, getMeshVertices, (int, int, const int*, double*), (const));
        MOCK_METHOD(void, getMeshVertexIDsFromPositions, (int, int, const double*, int*), (const));

        MOCK_METHOD(bool, hasData, (const std::string&, int ), (const));
        MOCK_METHOD(int, getDataID, ( const std::string&, int), (const));

        MOCK_METHOD(void, writeBlockVectorData, (int, int, const int*, double*));
        MOCK_METHOD(void, writeBlockScalarData, (int, int, const int*, double*));

        MOCK_METHOD(void, readBlockVectorData, (int, int, const int*, double*), (const));
        MOCK_METHOD(void, readBlockScalarData, (int, int, const int*, double*), (const));



    };
    namespace constants {
        struct mockConstants{
            MOCK_METHOD(const std::string&, initialData, ());
            MOCK_METHOD(const std::string&, writeIterationCheckpoint, ());
            MOCK_METHOD(const std::string&, readIterationCheckpoint, ());
        };
        // @brief Name of action for writing initial data.
        const std::string& actionWriteInitialData();

        // @brief Name of action for writing iteration checkpoint
        const std::string& actionWriteIterationCheckpoint();

        // @brief Name of action for reading iteration checkpoint.
        const std::string& actionReadIterationCheckpoint();
    } // namespace constants

}

#endif
