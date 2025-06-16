#!/bin/bash
set -e

BUILD_DIR="build"
MAIN_EXEC="${BUILD_DIR}/liverun"
TEST_EXEC="${BUILD_DIR}/tests/liverun_tests"

echo "Cleaning previous build..."
rm -rf "${BUILD_DIR}"

echo "Running CMake configure and build..."
cmake -S . -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}"

# Run tests
if [[ -f "${TEST_EXEC}" ]]; then
    echo -e "\n Running tests via CTest:"
    cd "${BUILD_DIR}/tests"
    ctest --output-on-failure
    cd ..
else
    echo -e "\n❌ Test binary not found at ${TEST_EXEC}"
fi
