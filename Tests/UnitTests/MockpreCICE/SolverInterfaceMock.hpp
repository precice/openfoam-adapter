//
// Created by keefe on 3/2/20.
//

#pragma once
#ifndef SolverInterface_H
#define SolverInterface_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

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
            solverConstructor();
        }
        MOCK_METHOD(void, solverConstructor, ());

        MOCK_METHOD(void, configure, (std::string&));

        double initialize();
        MOCK_METHOD(double, initializeMock, ());

        MOCK_METHOD(void, initializeData, ());

        MOCK_METHOD(double, advance, (double));
        MOCK_METHOD(void, finalize, ());
        MOCK_METHOD(int, getDimensions, (), (const));
        // Auxiliary methods
        MOCK_METHOD(bool, isCouplingOngoing, ());
        MOCK_METHOD(bool, isReadDataAvailable, (), (const));
        MOCK_METHOD(bool, isWriteDataRequired, (double), (const));
        MOCK_METHOD(bool, isTimestepComplete, (), (const));

        bool isActionRequired(const std::string& actionName) const;
        MOCK_METHOD(bool, isActionRequiredMock, (const std::string&), (const));
        MOCK_METHOD(void, fulfilledAction, (const std::string&), (const));

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
