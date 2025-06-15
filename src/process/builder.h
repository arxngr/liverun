#pragma once
#include "../liverun.h"
#include "manager.h"

namespace livrn {
class ProcessBuilder {
private:
  livrn::ProcessManager &processManager;

public:
  explicit ProcessBuilder(ProcessManager &pm);

  bool compileSync(const std::string &cmd);
};
} // namespace livrn
