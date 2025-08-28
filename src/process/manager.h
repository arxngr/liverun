#pragma once

#include "../cmd/command.h"
#include "../config.h"
#include "../liverun.h"
#include "../util/parser.h"

namespace livrn {
class ProcessManager {
private:
  pid_t childPid = -1;
  pid_t compilePid = -1;

  void killProcessGracefully(pid_t &pid, const std::string &processName);

public:
  ~ProcessManager();
  void cleanup() noexcept;

  void killChild();
  void killCompileProcess();

  bool startProcess(const std::vector<std::string> &args);
  bool startInterpreter(const std::string &interpreter,
                        const std::string &script);
  bool startBinary(const std::string &binary);
  bool startCommand(const std::string &cmd);
  bool authenticatedUser();

  bool isChildRunning() const;
};

} // namespace livrn
