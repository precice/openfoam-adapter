//
// Created by keefe on 27/1/20.
// This file mocks the Interface.H file in the OpenFOAM preCICE adapter.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include "../MockFOAM/fvMeshMock.h"
#include "Tests/UnitTests/MockpreCICE/SolverInterfaceMock.hpp"

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
        MOCK_METHOD(void, createBuffer, ());
        MOCK_METHOD(void, readCouplingData, ());
        MOCK_METHOD(void, writeCouplingData, ());
    };
}

#endif //INTERFACE_H
