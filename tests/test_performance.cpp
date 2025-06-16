#include "../src/process/monitor.h"
#include "test_helpers.h"
#include <gtest/gtest.h>
#include <iostream>

class PerformanceTest : public ::testing::Test {
protected:
  void SetUp() override { TestEnvironment::SetUpTestDirectory(); }

  void TearDown() override { TestEnvironment::TearDownTestDirectory(); }
};

TEST_F(PerformanceTest, FileSystemScanPerformance) {
  // Create many files
  const int num_files = 100;
  for (int i = 0; i < num_files; ++i) {
    TestEnvironment::createTestFile("file" + std::to_string(i) + ".cpp",
                                    "int main" + std::to_string(i) +
                                        "() { return 0; }");
  }

  livrn::ProcessMonitor monitor;

  auto start = std::chrono::high_resolution_clock::now();
  monitor.scanDirectory(".");
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // Should complete within reasonable time (adjust threshold as needed)
  EXPECT_LT(duration.count(), 1000); // Less than 1 second

  std::cout << "Scanned " << num_files << " files in " << duration.count()
            << "ms" << std::endl;
}

TEST_F(PerformanceTest, ChangeDetectionPerformance) {
  const int num_files = 50;
  for (int i = 0; i < num_files; ++i) {
    TestEnvironment::createTestFile("perf" + std::to_string(i) + ".cpp",
                                    "content");
  }

  livrn::ProcessMonitor monitor;
  monitor.scanDirectory(".");

  auto start = std::chrono::high_resolution_clock::now();

  // Check for changes multiple times
  for (int i = 0; i < 10; ++i) {
    monitor.hasAnyFileChanged();
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Should be very fast
  EXPECT_LT(duration.count(), 10000); // Less than 10ms

  std::cout << "10 change detection cycles took " << duration.count() << "μs"
            << std::endl;
}
