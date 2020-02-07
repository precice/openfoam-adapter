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
using namespace testing;
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
                    EXPECT_CALL(*this, configureMock(_)).WillOnce(Return(true));
                    isConfigureMock = true;
                }
                return configureMock(adapterConfig);
            };

            MOCK_METHOD(void, addWritersMock, (std::string, Interface *));
            void addWriters(std::string writer_name, Interface* interface){
                if (!isAddWritersMock){
                    EXPECT_CALL(*this, addWritersMock(_, _)).Times(1);
                    isAddWritersMock = true;
                }
                addWritersMock(std::move(writer_name), interface);
            }

            MOCK_METHOD(void, addReadersMock, (std::string, Interface *));
            void addReaders(std::string reader_name, Interface* interface){
                if(!isAddReadersMock){
                    EXPECT_CALL(*this, addReadersMock(_, _)).Times(2);
                    isAddReadersMock = true;
                }
                addReadersMock(std::move(reader_name), interface);
            }
        };
    }
}
#endif //CHT_H
