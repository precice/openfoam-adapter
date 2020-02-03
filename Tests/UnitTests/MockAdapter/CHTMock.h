//
// Created by keefe on 26/1/20.
//

#ifndef CHT_H
#define CHT_H

#include <yaml-cpp/node/node.h>
#include "../MockFOAM/fvMeshMock.h"
#include "../MockAdapter/InterfaceMock.h"

namespace preciceAdapter {
    namespace CHT {
        class ConjugateHeatTransfer{
        public:
            explicit ConjugateHeatTransfer(Foam::fvMesh const&){};
            ~ConjugateHeatTransfer()= default;

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
#endif //CHT_H
