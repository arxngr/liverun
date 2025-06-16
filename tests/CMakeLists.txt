cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent to download GTest with GMock
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

# Prevent override of parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(liverun_tests
    test_main.cpp
    test_validator.cpp
    test_command.cpp
    test_process_monitor.cpp
    test_process_manager.cpp
    test_builder.cpp
    test_performance.cpp
)

target_link_libraries(liverun_tests
    PRIVATE
        livern
        GTest::gtest_main
        GTest::gmock
)

# Add tests
add_test(NAME ValidatorTest COMMAND liverun_tests --gtest_filter=ValidatorTest.*)
add_test(NAME CommandTest COMMAND liverun_tests --gtest_filter=CommandTest.*)
add_test(NAME ProcessMonitorTest COMMAND liverun_tests --gtest_filter=ProcessMonitorTest.*)
add_test(NAME ProcessManagerTest COMMAND liverun_tests --gtest_filter=ProcessManagerIntegrationTest.*)
add_test(NAME BuilderTest COMMAND liverun_tests --gtest_filter=BuilderTest.*)
add_test(NAME PerformanceTests COMMAND liverun_tests --gtest_filter=PerformanceTest.*)

set_tests_properties(ValidatorTest PROPERTIES TIMEOUT 30)
set_tests_properties(CommandTest PROPERTIES TIMEOUT 10)
set_tests_properties(ProcessMonitorTest PROPERTIES TIMEOUT 30)
set_tests_properties(ProcessManagerTest PROPERTIES TIMEOUT 60)
set_tests_properties(BuilderTest PROPERTIES TIMEOUT 120)
set_tests_properties(PerformanceTests PROPERTIES TIMEOUT 60)

target_compile_options(liverun_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -g
    -O0
    --coverage
)

target_link_options(liverun_tests PRIVATE --coverage)

