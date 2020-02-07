//
// Created by keefe on 26/1/2020.
// This file mocks the CHT.H file in the OpenFOAM preCICE adapter.
//

#ifndef CHT_H
#define CHT_H

#include <yaml-cpp/node/node.h>

#include <utility>
#include "../MockFOAM/fvMeshMock.h"
#include "../MockAdapter/InterfaceMock.h"

namespace preciceAdapter {
    namespace CHT {
        class ConjugateHeatTransfer{
        public:
            static bool isConfigureMock, isAddWritersMock, isAddReadersMock;
            explicit ConjugateHeatTransfer(Foam::fvMesh const&){
                isConfigureMock = isAddWritersMock = isAddReadersMock = false;
            };
            ~ConjugateHeatTransfer()= default;

            // Explicit method created for 'configure' to allow run-time modification of mocked
            // configureMock() method behavior.

            MOCK_METHOD(bool, configureMock, (const YAML::Node&));
            bool configure(const YAML::Node& adapterConfig){
                if(!isConfigureMock){
                    EXPECT_CALL(*this, configureMock(testing::_)).WillOnce(testing::Return(true));
                    isConfigureMock = true;
                }
                return configureMock(adapterConfig);
            };

            MOCK_METHOD(void, addWritersMock, (std::string, Interface *));
            void addWriters(std::string writer_name, Interface* interface){
                std::string writer = "Temperature";
                if (!isAddWritersMock){
                    EXPECT_CALL(*this, addWritersMock(writer, interface)).Times(1);
                    isAddWritersMock = true;
                }
                addWritersMock(std::move(writer_name), interface);
            }

            MOCK_METHOD(void, addReadersMock, (std::string, Interface *));
            void addReaders(std::string writer_name, Interface* interface){
                std::string reader = "Heat-Flux";
                if(!isAddReadersMock){
                    EXPECT_CALL(*this, addWritersMock(reader, interface)).Times(1);
                    isAddReadersMock = true;
                }
                addWritersMock(std::move(writer_name), interface);
            }
        };
    }
}
#endif //CHT_H
