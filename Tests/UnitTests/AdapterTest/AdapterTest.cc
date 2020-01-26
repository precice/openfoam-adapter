#include "../../../Adapter.H"
#include "gtest/gtest.h"

class AdapterTest : public ::testing::Test {
 protected:
  void SetUp() override {
      preciceAdapter::Adapter adapter_;
  }

  void TearDown() override {
      
  }

private:
    preciceAdapter::Adapter adapter;

};


TEST(FactorialTest, Negative) {
// This test is named "Negative", and belongs to the "FactorialTest"
// test case.
EXPECT_EQ(1, 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
