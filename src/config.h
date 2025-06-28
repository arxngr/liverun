#ifndef LIVRN_CONFIG_H
#define LIVRN_CONFIG_H
#include "liverun.h"

namespace livrn {
namespace Config {
const std::unordered_set<std::string> ALLOWED_COMMANDS = {
    "make",    "dlv",   "go",  "gcc",  "g++",  "clang",
    "clang++", "cargo", "npm", "yarn", "cmake"};

const std::unordered_set<std::string> ALLOWED_EXTENSIONS = {
    ".c",  ".cpp", ".cc", ".cxx", ".h", ".hpp",
    ".go", ".rs",  ".js", ".ts",  ".py"};

const size_t MAX_COMMAND_LENGTH = 1024;
const size_t MAX_PATH_LENGTH = 512;
const size_t MAX_ARG_LENGTH = 256;
const int GRACEFUL_SHUTDOWN_TIMEOUT_MS = 3000;
const int POLL_INTERVAL_MS = 500;
const int SHUTDOWN_DELAY_MS = 500;
} // namespace Config
} // namespace livrn

#endif
