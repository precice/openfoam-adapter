//
// Created by keefe on 26/1/20.
// This file mocks the FSI.H file in the OpenFOAM preCICE adapter.
//

#ifndef FSI_H
#define FSI_H

#include <yaml-cpp/node/node.h>
#include "../MockFOAM/fvMeshMock.h"
#include "../MockFOAM/timeMock.h"
#include "../MockAdapter/InterfaceMock.h"

namespace preciceAdapter {
    namespace FSI {
        class FluidStructureInteraction{
        public:
            FluidStructureInteraction(Foam::fvMesh const&, Foam::Time const&){};
            ~FluidStructureInteraction()= default;

            // Explicit method created for 'configure' to allow run-time modification of mocked
            // configureMock() method behavior.
            MOCK_METHOD(bool, configureMock, (const YAML::Node&));
            bool configure(const YAML::Node& adapterConfig){
                ON_CALL(*this, configureMock(testing::_)).WillByDefault(testing::Return(true));
                return configureMock(adapterConfig);
            };

            MOCK_METHOD(void, addWriters, (std::string, Interface *));
            MOCK_METHOD(void, addReaders, (std::string, Interface *));
        };
    }
}
#endif //FSI_H
