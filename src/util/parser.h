#pragma once
#include "../config.h"

namespace livrn {
class Parser {
public:
  static bool isPathSafe(const std::string &path);
  static bool isCommandSafe(const std::string &cmd);
  static bool isBinaryFile(const fs::path &path);
};
}; // namespace livrn
