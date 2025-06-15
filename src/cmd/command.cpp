#include "command.h"

namespace livrn {

std::vector<std::string> Command::parseCommand(const std::string &cmd) {
  std::vector<std::string> args;
  std::istringstream iss(cmd);
  std::string arg;

  while (iss >> arg) {
    if (arg.length() > livrn::Config::MAX_ARG_LENGTH) {
      continue;
    }

    // Remove quotes if present
    if (arg.length() >= 2 && arg.front() == '"' && arg.back() == '"') {
      arg = arg.substr(1, arg.length() - 2);
    }

    args.push_back(arg);
  }

  return args;
}
} // namespace livrn
