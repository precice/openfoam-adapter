#ifndef GTEST_ADAPTERUNITTEST_H
#define GTEST_ADAPTERUNITTEST_H

#include "Tests/UnitTests/MockFOAM/timeMock.h"
#include "../MockFOAM/fvMeshMock.h"
#include "../MockFOAM/messageStreamMock.h"
#include "Tests/UnitTests/MockAdapter/CHTMock.h"
#include "Tests/UnitTests/MockAdapter/FSIMock.h"
#include "../MockAdapter/InterfaceMock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <exception>


#define private public

#include "../../../Adapter.H"

using namespace testing;

const std::string INFO_LABEL = "info";
const std::string ERROR_LABEL = "error-deferred";

const std::string STANDARD_YAML_MESSAGE = "Reading the adapter's YAML configuration file "
                                          "./TestFolder/precice-adapter-config.yml...";
const std::string STANDARD_LOAD_MESSAGE = "preCICE was configured and initialized";

class AdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::string load_message = "The preciceAdapter was loaded.";
        EXPECT_CALL(adapterInfo_, adapterInfo(load_message, INFO_LABEL));
        adapter_ = new preciceAdapter::Adapter(runTime_, mesh_);
        std::cout << "SetUp: Creating new precice adapter\n";
    }

//
    void TearDown() override {
        delete adapter_;
    }

    // - AdapterInfo object to track logging
    adapterInfoMock adapterInfo_;

    // - OpenFOAM runTime object
    Foam::Time runtime;
    const Foam::Time &runTime_ = runtime;

    //- OpenFOAM fvMesh object
    Foam::fvMesh mesh;
    const Foam::fvMesh &mesh_ = mesh;
    preciceAdapter::Adapter *adapter_ = nullptr;
};

TEST_F(AdapterTest, ConfigFileCheckTest_MissingFile) {
    // Ensure configFileCheck() throws std::runtime_error if handed non-existent file
    // Checks that handing non-existent file to configFileCheck results in std::runtime_error thrown
    ASSERT_THROW(adapter_->configFileCheck("./TestFolder/FAIL_TEST_XML/missingfile.yml"),
            std::runtime_error);
}

TEST_F(AdapterTest, ConfigFileCheckTest_MissingParticipant) {
    // Ensures configFileCheck() returns false if participant is missing in precice-adapter-config.yml
    std::string error_message = "The 'participant' node is missing in "
                                "./TestFolder/FAIL_TEST_XML/precice-adapter-missing-participant.yml.";

    EXPECT_CALL(adapterInfo_, adapterInfo(error_message, ERROR_LABEL));
    ASSERT_FALSE(adapter_->
    configFileCheck("./TestFolder/FAIL_TEST_XML/precice-adapter-missing-participant.yml"));
}

TEST_F(AdapterTest, ConfigFileCheckTest_MissingPreciceFile) {
    // Ensures configFileCheck() returns false if precice-config-file is missing in precice-adapter-config.yml
    std::string error_message = "The 'precice-config-file' node is missing in "
                                "./TestFolder/FAIL_TEST_XML/precice-adapter-missing-precice-file.yml.";

    EXPECT_CALL(adapterInfo_, adapterInfo(error_message, ERROR_LABEL));
    ASSERT_FALSE(adapter_->configFileCheck("./TestFolder/FAIL_TEST_XML/precice-adapter-missing-precice-file.yml"));
}

TEST_F(AdapterTest, ConfigFileCheckTest_MissingInterfaces) {
    // Ensures configFileCheck() returns false if interfaces is missing in precice-adapter-config.yml
    std::string error_message = "The 'interfaces' node is missing in "
                                "./TestFolder/FAIL_TEST_XML/precice-adapter-missing-interfaces.yml.";

    EXPECT_CALL(adapterInfo_, adapterInfo(error_message, ERROR_LABEL));
    ASSERT_FALSE(adapter_->configFileCheck("./TestFolder/FAIL_TEST_XML/precice-adapter-missing-interfaces.yml"));
}

TEST_F(AdapterTest, ConfigFileCheckTest_MissingMesh) {
    // Ensures configFileCheck() returns false if mesh is missing in an interface in precice-adapter-config.yml
    std::string error_message = "The 'mesh' node is missing for the interface #1 in "
                                "./TestFolder/FAIL_TEST_XML/precice-adapter-missing-mesh.yml.";

    EXPECT_CALL(adapterInfo_, adapterInfo(error_message, ERROR_LABEL));
    ASSERT_FALSE(adapter_->configFileCheck("./TestFolder/FAIL_TEST_XML/precice-adapter-missing-mesh.yml"));
}

TEST_F(AdapterTest, ConfigFileCheckTest_MissingPatches) {
    // Ensures configFileCheck() returns false if patches is missing in an interface in precice-adapter-config.yml
    std::string error_message = "The 'patches' node is missing for the interface #1 in "
                                "./TestFolder/FAIL_TEST_XML/precice-adapter-missing-patches.yml.";

    EXPECT_CALL(adapterInfo_, adapterInfo(error_message, ERROR_LABEL));
    ASSERT_FALSE(adapter_->configFileCheck("./TestFolder/FAIL_TEST_XML/precice-adapter-missing-patches.yml"));
}

TEST_F(AdapterTest, ConfigFileCheckTest_Success) {
    // Ensures configFileCheck() returns true if passed working precice-adapter-config.yml
    ASSERT_TRUE(adapter_->configFileCheck("./TestFolder/precice-adapter-config.yml"));
}

TEST_F(AdapterTest, ConfigFileReadTest_MissingFile) {
    // Ensures configFileRead() throws std::runtime_error if handed non-existent file
    std::string read_message = "Reading the adapter's YAML configuration file "
                               "./DoesNotExist/../precice-adapter-config.yml...";

    EXPECT_CALL(adapterInfo_, adapterInfo(read_message, INFO_LABEL));
    EXPECT_CALL(runtime, processorCase()).WillOnce(Return(true));
    EXPECT_CALL(runtime, path()).WillOnce(Return("./DoesNotExist"));
    ASSERT_THROW(adapter_->configFileRead(), std::runtime_error);
}

TEST_F(AdapterTest, ConfigFileReadTest_BadFile) {
    // Ensures configFileRead() returns false if handed bad file.
    std::string read_message = "Reading the adapter's YAML configuration file "
                               "./TestFolder/FAIL_TEST_XML/precice-adapter-config.yml...";
    std::string error_message = "The 'participant' node is missing in "
                                "./TestFolder/FAIL_TEST_XML/precice-adapter-config.yml.";

    EXPECT_CALL(adapterInfo_, adapterInfo(error_message, ERROR_LABEL));
    EXPECT_CALL(adapterInfo_, adapterInfo(read_message, INFO_LABEL));
    EXPECT_CALL(runtime, processorCase()).WillOnce(Return(false));
    EXPECT_CALL(runtime, path()).WillOnce(Return("./TestFolder/FAIL_TEST_XML"));
    EXPECT_FALSE(adapter_->configFileRead());
}

// More extension configFileRead tests needed. Need to check for edge cases for settings states
// Need tests for FSI.H: true
TEST_F(AdapterTest, ConfigFileReadTest_Success) {
    // Ensure configFileRead() returns true if handed working file.
    EXPECT_CALL(adapterInfo_, adapterInfo(STANDARD_YAML_MESSAGE, INFO_LABEL));
    EXPECT_CALL(runtime, processorCase()).WillOnce(Return(false));
    EXPECT_CALL(runtime, path()).WillOnce(Return("./TestFolder"));
    EXPECT_TRUE(adapter_->configFileRead());

    // Ensure data in file is read accurately
    EXPECT_EQ(adapter_->preciceConfigFilename_, "./precice-config.xml");
    std::vector<struct preciceAdapter::Adapter::InterfaceConfig> interfaces = adapter_->interfacesConfig_;
    // Ensure contents of 'Fluid-Mesh' interface are correct
    EXPECT_EQ(interfaces[0].meshName, "Fluid-Mesh");
    EXPECT_EQ(interfaces[0].locationsType, "faceCenters");
    EXPECT_FALSE(interfaces[0].meshConnectivity);
    std::vector<std::string> patchNames1 = {"interface"};
    EXPECT_EQ(interfaces[0].patchNames, patchNames1);

    std::vector<std::string> readData1 = {"Heat-Flux"};
    std::vector<std::string> writeData1 = {"Temperature"};
    EXPECT_EQ(interfaces[0].readData, readData1);
    EXPECT_EQ(interfaces[0].writeData, writeData1);

    // Ensure contents of Fluid-2-Mesh' interfaces are correct
    EXPECT_EQ(interfaces[1].meshName, "Fluid-2-Mesh");
    EXPECT_EQ(interfaces[1].locationsType, "faceNodes");
    EXPECT_TRUE(interfaces[1].meshConnectivity);
    std::vector<std::string> patchNames2 = {"patch1", "patch2"};
    EXPECT_EQ(interfaces[1].patchNames, patchNames2);
    std::vector<std::string> writeData2 = {"Displacement"};
    EXPECT_EQ(interfaces[1].readData, writeData2);

    // Ensure final states of flags in adapter are correctly set
    EXPECT_FALSE(adapter_->subcyclingAllowed_);
    EXPECT_TRUE(adapter_->preventEarlyExit_);
    EXPECT_FALSE(adapter_->evaluateBoundaries_);
    EXPECT_TRUE(adapter_->disableCheckpointing_);

    EXPECT_TRUE(adapter_->CHTenabled_);
    EXPECT_FALSE(adapter_->FSIenabled_);
}

TEST_F(AdapterTest, ConfigureTest) {
    Foam::unwatchedIOdictionaryMock controlDict_;
    // Catch info level events
    EXPECT_CALL(adapterInfo_, adapterInfo(_, INFO_LABEL));
    // Specify standard yaml load and precice adapter load messages
    EXPECT_CALL(adapterInfo_, adapterInfo(STANDARD_YAML_MESSAGE, INFO_LABEL));
    EXPECT_CALL(adapterInfo_, adapterInfo(STANDARD_LOAD_MESSAGE, INFO_LABEL));

    EXPECT_CALL(runtime, processorCase()).WillOnce(Return(false));
    EXPECT_CALL(controlDict_, lookupOrDefault("adjustTimeStep", false)).WillOnce(Return(true));
    EXPECT_CALL(runtime, path()).WillOnce(Return("./TestFolder"));
    EXPECT_CALL(runtime, controlDict()).WillOnce(ReturnRef(controlDict_));
    EXPECT_CALL(runtime, value()).WillRepeatedly(testing::Return(Foam::TEST_TIME_STEP));
    EXPECT_CALL(runtime, timeIndex()).WillOnce(Return(1));
    EXPECT_CALL(runtime, setEndTime(Foam::GREAT)).Times(1);

    adapter_->configure();
}

TEST_F(AdapterTest, ExecuteTest){
    Foam::unwatchedIOdictionaryMock controlDict_;
    std::string precice_config = "precice-config.xml";
    Foam::classMock classMock_;
    Foam::timePathMock timePath_;

    // Catch info level events
    EXPECT_CALL(adapterInfo_, adapterInfo(_, INFO_LABEL)).Times(5);

    EXPECT_CALL(runtime, processorCase()).WillOnce(Return(false));
    EXPECT_CALL(controlDict_, lookupOrDefault("adjustTimeStep", false)).WillOnce(Return(true));
    EXPECT_CALL(runtime, path()).WillOnce(Return("./TestFolder"));
    EXPECT_CALL(runtime, controlDict()).WillOnce(ReturnRef(controlDict_));
    EXPECT_CALL(runtime, functionObjects()).WillRepeatedly(testing::ReturnRef(classMock_));
    EXPECT_CALL(runtime, timeIndex()).Times(2);
    EXPECT_CALL(runtime, setEndTime(Foam::GREAT)).Times(1);
    EXPECT_CALL(runtime, setEndTime(0)).Times(1);
    EXPECT_CALL(runtime, timePath()).WillOnce(testing::ReturnRef(timePath_));
    adapter_->configure();
    adapter_->execute();
}

TEST_F(AdapterTest, AdapterTest_AdjustTimeStepModifiableTrue_Test){
//    EXPECT_CALL(runtime, runTimeModifiable()).WillOnce(Return(true));
    Foam::Time time_;
    EXPECT_CALL(runtime, deltaT()).WillOnce(ReturnRef(time_));
    adapter_->adjustTimeStep();
}

TEST_F(AdapterTest, EndTest){
    adapter_->end();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif //#ifndef GTEST_ADAPTERUNITTEST_H