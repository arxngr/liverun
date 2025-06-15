#include "parser.h"

namespace livrn {
bool Parser::isPathSafe(const std::string &path) {
  try {
    fs::path p(path);
    fs::path canonical = fs::canonical(p.parent_path()) / p.filename();
    fs::path cwd = fs::current_path();
    auto rel = fs::relative(canonical, cwd);

    if (rel.string().substr(0, 2) == "..") {
      return false;
    }

    if (canonical.string().length() > livrn::Config::MAX_PATH_LENGTH) {
      return false;
    }

    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool Parser::isCommandSafe(const std::string &cmd) {
  if (cmd.empty() || cmd.length() > livrn::Config::MAX_COMMAND_LENGTH) {
    return false;
  }

  const std::string dangerous_chars = ";|&`$(){}[]<>*?";
  if (cmd.find_first_of(dangerous_chars) != std::string::npos) {
    return false;
  }

  std::istringstream iss(cmd);
  std::string first_word;
  iss >> first_word;

  return livrn::Config::ALLOWED_COMMANDS.find(first_word) !=
         livrn::Config::ALLOWED_COMMANDS.end();
}

bool Parser::isBinaryFile(const fs::path &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open())
    return false;

  char buffer[512];
  file.read(buffer, sizeof(buffer));
  std::streamsize bytesRead = file.gcount();

  for (std::streamsize i = 0; i < bytesRead; ++i) {
    unsigned char c = buffer[i];
    if (c < 9 || (c > 13 && c < 32) || c == 127) {
      return true;
    }
  }
  return false;
}
} // namespace livrn
