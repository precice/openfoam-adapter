//
// Created by keefe on 26/1/20.
//

#ifndef FSI_H
#define FSI_H

#include <yaml-cpp/node/node.h>
#include "../MockFOAM/fvMeshMock.h"
#include "../MockFOAM/timeMock.h"
#include "../MockAdapter/InterfaceMock.h"
class FSIMock {

};

namespace preciceAdapter {
    namespace FSI {
        class FluidStructureInteraction{
        public:
            FluidStructureInteraction(Foam::fvMesh const&, Foam::Time const&){};
            ~FluidStructureInteraction()= default;
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
