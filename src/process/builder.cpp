#include "builder.h"
#include "../cmd/command.h"
#include "../util/parser.h"

namespace livrn {
ProcessBuilder::ProcessBuilder(ProcessManager &pm) { (void)pm; }

bool ProcessBuilder::compileSync(const std::string &cmd) {
  if (!livrn::Parser::isCommandSafe(cmd)) {
    std::cerr << "[livrn] Unsafe command rejected: " << cmd << std::endl;
    return false;
  }

  std::cout << "[livrn] Compiling: " << cmd << std::endl;

  auto args = livrn::Command::parseCommand(cmd);
  if (args.empty())
    return false;

  pid_t pid = fork();
  if (pid == 0) {
    std::vector<char *> argv;
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    execvp(argv[0], argv.data());
    exit(1);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
  }

  return false;
}

} // namespace livrn
