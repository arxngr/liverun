#pragma once
#include "../config.h"
#include "../liverun.h"

namespace livrn {
class Command {
public:
  static std::vector<std::string> parseCommand(const std::string &cmd);
};
} // namespace livrn
