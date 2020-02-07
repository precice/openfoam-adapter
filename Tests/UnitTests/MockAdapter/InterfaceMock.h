//
// Created by keefe on 27/1/20.
// This file mocks the Interface.H file in the OpenFOAM preCICE adapter.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include "../MockFOAM/fvMeshMock.h"
#include "../MockpreCICE/SolverInterfaceMock.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace preciceAdapter {
    class Interface {
    public:
        Interface
                (
                        precice::SolverInterface & precice,
                        const Foam::fvMesh& mesh,
                        std::string& meshName,
                        std::string& locationsType,
                        std::vector<std::string>& patchNames,
                        bool meshConnectivity
                ){};
        ~Interface()=default;
        MOCK_METHOD(void, createBufferMock, ());
        void createBuffer(){
            EXPECT_CALL(*this, createBufferMock()).Times(1);
            createBufferMock();
        }

        MOCK_METHOD(void, readCouplingDataMock, ());
        void readCouplingData(){
            ON_CALL(*this, readCouplingDataMock());
        }
        MOCK_METHOD(void, writeCouplingDataMock, ());

        void writeCouplingData(){
            ON_CALL(*this, writeCouplingDataMock());
        }
    };
}

#endif //INTERFACE_H
