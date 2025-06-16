#include "test_helpers.h"
#include <gtest/gtest.h>

class ApplicationTest : public ::testing::Test {
protected:
  void SetUp() override { TestEnvironment::SetUpTestDirectory(); }

  void TearDown() override { TestEnvironment::TearDownTestDirectory(); }
};

TEST_F(ApplicationTest, ParseArguments) {
  Application app;

  // Test invalid argument count
  {
    char *argv[] = {(char *)"hotreload"};
    int result = app.run(1, argv);
    EXPECT_EQ(result, 1);
  }

  // Test unknown mode
  {
    char *argv[] = {(char *)"hotreload", (char *)"unknown_mode"};
    int result = app.run(2, argv);
    EXPECT_EQ(result, 1);
  }
}
