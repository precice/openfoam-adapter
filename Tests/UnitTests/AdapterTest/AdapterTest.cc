#include "../MockFOAM/timeMock.h"
#include "../MockFOAM/fvMeshMock.h"
#include "../MockFOAM/fvCFDMock.h"
#include "../MockFOAM/messageStreamMock.h"
#include "../MockAdapter/CHTMock.h"
#include "../MockAdapter/FSIMock.h"
#include "../MockAdapter/InterfaceMock.h"

#include "../../../Adapter.H"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::Return;
using ::testing::ReturnRef;

class AdapterTest : public ::testing::Test
{
 protected:
  void SetUp() override {
//    adapter_= std::make_shared<preciceAdapter::Adapter>(runTime_, mesh_);
      std::cout << "Creating new precice adapter\n";
//    adapter_ = new preciceAdapter::Adapter(runTime_, mesh_);
      adapter_ = new preciceAdapter::Adapter(runTime_, mesh_);
  }
//
  void TearDown() override {
      delete adapter_;
  }

    // - OpenFOAM runTime object
    Foam::Time runtime;
    const Foam::Time& runTime_ = runtime;

    //- OpenFOAM fvMesh object
    Foam::fvMesh mesh;
    const Foam::fvMesh& mesh_ = mesh;
    preciceAdapter::Adapter* adapter_ = nullptr;
};


TEST_F(AdapterTest, SampleTest) {
// This test is named "Negative", and belongs to the "FactorialTest"
// test case.
    EXPECT_EQ(1, 1);
}

TEST_F(AdapterTest, ConfigureTest)
{
//    std::cout << "Out";
    Foam::unwatchedIOdictionaryMock controlDict_;
    std::string precice_config = "precice-config.xml";

    EXPECT_CALL(runtime, processorCase()).WillOnce(Return(false));
    EXPECT_CALL(controlDict_, lookupOrDefault("adjustTimeStep", false)).WillOnce(Return(true));
    EXPECT_CALL(runtime, path()).WillOnce(Return("./"));
    EXPECT_CALL(runtime, controlDict()).WillOnce(ReturnRef(controlDict_));
    adapter_->configure();

}
//
//TEST_F(AdapterTest, ExecuteTest){
//    adapter_->execute();
//}
//
//TEST_F(AdapterTest, AdjustTimeStepTest){
//    adapter_->adjustTimeStep();
//}
//
//TEST_F(AdapterTest, EndTest){
//    adapter_->end();
//}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
