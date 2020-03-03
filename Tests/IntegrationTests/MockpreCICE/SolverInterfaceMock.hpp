//
// Created by keefe on 3/2/20.
//

#pragma once
#ifndef SolverInterface_H
#define SolverInterface_H

#include "fakeit.hpp"
using namespace fakeit;

namespace precice{
    class SolverInterface{
    public:
        const static std::string readIterationCheckpoint;
        const static std::string writeIterationCheckpoint;
        const static std::string initialDataName;
        static int count;

        virtual void configure(std::string&) = 0;
        virtual double initialize() = 0;
        virtual void initializeData() = 0;
        virtual double advance(double) = 0;
        virtual void finalize() = 0;
        virtual int getDimensions() const = 0;
        
        
        // Auxiliary methods
        virtual bool isCouplingOngoing() = 0;
        virtual bool isReadingDataAvailable() const = 0;
        virtual bool isReadDataAvailable() const = 0;
        virtual bool isWriteDataRequired(double) const = 0;
        virtual bool isTimeWindowComplete() const = 0;

        virtual bool isActionRequired(const std::string) const = 0;
        virtual void markActionFulfilled(const std::string&) const = 0;

        virtual bool hasMesh(const std::string&) const = 0;
        // Mesh Methods
        
        virtual std::set<int> getMeshIDS() const = 0;
        virtual int getMeshID(std::string&) const = 0;
        virtual int setMeshVertex(int, const double*) = 0;
        virtual int getMeshVertexSize(int) const = 0;
        
        virtual void setMeshVertices(int, int, const double*, int*) = 0;
        virtual void getMeshVertices(int, int, const int*, double*) const = 0;
        virtual void getMeshVertexIDsFromPositions(int, int, const double*, int*) const = 0;
        virtual void setMeshTriangleWithEdges(int, int, int, int) const = 0;

        virtual bool hasData(const std::string&, int) const = 0;
        virtual int getDataID(const std::string&, int) const = 0;

        virtual void writeBlockVectorData(int, int, const int*, double*) = 0;
        virtual void writeBlockScalarData(int, int, const int*, double*) = 0;

        virtual void readBlockVectorData(int, int, const int*, double*) const = 0;
        virtual void readBlockScalarData(int, int, const int*, double*) const = 0;

    };
    namespace constants {
        // struct mockConstants{
        //     MOCK_METHOD(const std::string&, initialData, ());
        //     MOCK_METHOD(const std::string&, writeIterationCheckpoint, ());
        //     MOCK_METHOD(const std::string&, readIterationCheckpoint, ());
        // };
        // @brief Name of action for writing initial data.
        const std::string& actionWriteInitialData(); 

        // @brief Name of action for writing iteration checkpoint
        const std::string& actionWriteIterationCheckpoint();

        // @brief Name of action for reading iteration checkpoint.
        const std::string& actionReadIterationCheckpoint();
    } // namespace constants
}

namespace integrationtest{
    void writeCouplingTest(Mock<precice::SolverInterface>& mock);
    void configureTest(Mock<precice::SolverInterface>& mock);
}



#endif
