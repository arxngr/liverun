#include "../src/process/builder.h"
#include "../src/process/manager.h"
#include "test_helpers.h"
#include <gtest/gtest.h>

class BuilderTest : public ::testing::Test {
protected:
  livrn::ProcessManager processManager;
  livrn::ProcessBuilder compiler{processManager};

  void SetUp() override { TestEnvironment::SetUpTestDirectory(); }

  void TearDown() override { TestEnvironment::TearDownTestDirectory(); }
};

TEST_F(BuilderTest, CompileSimpleProgram) {
  TestEnvironment::createTestFile(
      "hello.cpp", "#include <iostream>\n"
                   "int main() {\n"
                   "    std::cout << \"Hello, World!\" << std::endl;\n"
                   "    return 0;\n"
                   "}\n");

  bool success = compiler.compileSync("g++ hello.cpp -o hello");

  if (system("which g++ > /dev/null 2>&1") == 0) {
    EXPECT_TRUE(success);
    EXPECT_TRUE(fs::exists("hello"));
  } else {
    GTEST_SKIP() << "g++ not available in test environment";
  }
}

TEST_F(BuilderTest, CompileInvalidProgram) {
  TestEnvironment::createTestFile(
      "invalid.cpp",
      "#include <iostream>\n"
      "int main() {\n"
      "    std::cout << \"Hello, World!\" << std::endl\n" // Missing semicolon
      "    return 0;\n"
      "}\n");

  if (system("which g++ > /dev/null 2>&1") == 0) {
    bool success = compiler.compileSync("g++ invalid.cpp -o invalid");
    EXPECT_FALSE(success);
  } else {
    GTEST_SKIP() << "g++ not available in test environment";
  }
}

TEST_F(BuilderTest, CompileUnsafeCommand) {
  bool success = compiler.compileSync("g++ hello.cpp -o hello; rm -rf /");
  EXPECT_FALSE(success);
}
