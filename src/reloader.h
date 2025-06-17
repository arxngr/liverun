#pragma once
#include "liverun.h"
#include "process/builder.h"
#include "process/manager.h"
#include "process/monitor.h"

namespace livrn {

class Reloader {
private:
  ProcessMonitor monitor;
  ProcessManager processManager;
  ProcessBuilder compiler;

public:
  Reloader();
  ~Reloader();

  void initialize();

  int runInterpretMode(const std::string &interpreter,
                       const std::string &script);
  int runCompileMode(const std::string &binary, const std::string &compileCmd);
  int runCommandMode(const std::vector<std::string> &commands);
};

} // namespace livrn
