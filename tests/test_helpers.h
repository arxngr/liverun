#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <thread>

namespace fs = std::filesystem;

class TestEnvironment {
public:
  static void SetUpTestDirectory() {
    fs::create_directory("test_dir");
    fs::current_path("test_dir");
  }

  static void TearDownTestDirectory() {
    fs::current_path("..");
    fs::remove_all("test_dir");
  }

  static void createTestFile(const std::string &filename,
                             const std::string &content) {
    std::ofstream file(filename);
    file << content;
    file.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  static void modifyTestFile(const std::string &filename,
                             const std::string &content) {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10)); // Ensure timestamp difference
    std::ofstream file(filename);
    file << content;
    file.close();
  }
};

class MockProcessManager {
public:
  MOCK_METHOD(void, killChild, ());
  MOCK_METHOD(void, killCompileProcess, ());
  MOCK_METHOD(bool, startProcess, (const std::vector<std::string> &args));
  MOCK_METHOD(bool, startInterpreter,
              (const std::string &interpreter, const std::string &script));
  MOCK_METHOD(bool, startBinary, (const std::string &binary));
  MOCK_METHOD(bool, startCommand, (const std::string &cmd));
  MOCK_METHOD(bool, isChildRunning, (), (const));
  MOCK_METHOD(void, cleanup, ());
};
