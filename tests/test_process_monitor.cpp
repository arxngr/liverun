#include "../src/process/monitor.h"
#include "test_helpers.h"
#include <gtest/gtest.h>

class ProcessMonitorTest : public ::testing::Test {
protected:
  livrn::ProcessMonitor monitor;

  void SetUp() override { TestEnvironment::SetUpTestDirectory(); }

  void TearDown() override { TestEnvironment::TearDownTestDirectory(); }
};

TEST_F(ProcessMonitorTest, ScanDirectory) {
  TestEnvironment::createTestFile("test1.cpp", "content1");
  TestEnvironment::createTestFile("test2.h", "content2");
  TestEnvironment::createTestFile("test3.txt", "content3");
  TestEnvironment::createTestFile("test4.py", "content4");

  fs::create_directory("subdir");
  TestEnvironment::createTestFile("subdir/test5.cpp", "content5");

  monitor.scanDirectory(".");

  EXPECT_FALSE(monitor.hasAnyFileChanged()); // No changes yet

  TestEnvironment::modifyTestFile("test1.cpp", "modified content1");
  EXPECT_TRUE(monitor.hasAnyFileChanged());

  EXPECT_FALSE(monitor.hasAnyFileChanged());
}

TEST_F(ProcessMonitorTest, DetectFileChanges) {
  TestEnvironment::createTestFile("monitor_test.cpp", "original content");
  monitor.scanDirectory(".");

  EXPECT_FALSE(monitor.hasAnyFileChanged());

  TestEnvironment::modifyTestFile("monitor_test.cpp", "new content");
  EXPECT_TRUE(monitor.hasAnyFileChanged());

  TestEnvironment::modifyTestFile("monitor_test.cpp", "newer content");
  EXPECT_TRUE(monitor.hasAnyFileChanged());
}

TEST_F(ProcessMonitorTest, IgnoreUnsafeFiles) {
  TestEnvironment::createTestFile("safe.cpp", "content");
  monitor.scanDirectory(".");

  TestEnvironment::modifyTestFile("safe.cpp", "new content");
  EXPECT_TRUE(monitor.hasAnyFileChanged());
}
