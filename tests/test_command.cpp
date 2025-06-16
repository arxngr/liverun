#include "../src/cmd/command.h"
#include "test_helpers.h"
#include <gtest/gtest.h>

class CommandTest : public ::testing::Test {};

TEST_F(CommandTest, ParseSimpleCommand) {
  auto result = livrn::Command::parseCommand("gcc main.cpp -o main");

  ASSERT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], "gcc");
  EXPECT_EQ(result[1], "main.cpp");
  EXPECT_EQ(result[2], "-o");
  EXPECT_EQ(result[3], "main");
}

// FIXME
//  TEST_F(CommandTest, ParseCommandWithQuotes) {
//    auto result =
//        livrn::Command::parseCommand("gcc \"main file.cpp\" -o \"output
//        file\"");
//
//    ASSERT_EQ(result.size(), 4);
//    EXPECT_EQ(result[0], "gcc");
//    EXPECT_EQ(result[1], "main file.cpp");
//    EXPECT_EQ(result[2], "-o");
//    EXPECT_EQ(result[3], "output file");
//  }
//
TEST_F(CommandTest, ParseCommandWithLongArgs) {
  std::string longArg(livrn::Config::MAX_ARG_LENGTH + 1, 'a');
  auto result = livrn::Command::parseCommand("gcc " + longArg + " main.cpp");

  // Long argument should be skipped
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], "gcc");
  EXPECT_EQ(result[1], "main.cpp");
}

TEST_F(CommandTest, ParseEmptyCommand) {
  auto result = livrn::Command::parseCommand("");
  EXPECT_TRUE(result.empty());
}
