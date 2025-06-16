#include "../src/process/manager.h"
#include "test_helpers.h"
#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

class ProcessManagerTest : public ::testing::Test {
protected:
  livrn::ProcessManager processManager;

  void SetUp() override { TestEnvironment::SetUpTestDirectory(); }

  void TearDown() override {
    processManager.cleanup(); // Ensure cleanup
    TestEnvironment::TearDownTestDirectory();
  }
};

TEST_F(ProcessManagerTest, StartAndKillSimpleProcess) {
  bool started = processManager.startProcess({"sleep", "10"});
  EXPECT_TRUE(started);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  processManager.killChild();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST_F(ProcessManagerTest, StartInvalidProcess) {
  bool started = processManager.startProcess({"nonexistent_command_12345"});
  EXPECT_TRUE(started);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  processManager.killChild();
}

TEST_F(ProcessManagerTest, StartBinary) {
  TestEnvironment::createTestFile("test_script.sh", "#!/bin/bash\nsleep 5\n");
  fs::permissions("test_script.sh", fs::perms::owner_exec,
                  fs::perm_options::add);

  bool started = processManager.startBinary("./test_script.sh");
  EXPECT_TRUE(started);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  processManager.killChild();
}

TEST_F(ProcessManagerTest, StartInterpreter) {
  TestEnvironment::createTestFile("test.py", "import time\ntime.sleep(5)\n");

  bool started = processManager.startInterpreter("python3", "test.py");
  EXPECT_TRUE(started);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  processManager.killChild();
}
