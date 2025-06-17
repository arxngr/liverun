#pragma once
#include "../liverun.h"
#include "manager.h"

namespace livrn {
class ProcessBuilder {
public:
  explicit ProcessBuilder(ProcessManager &pm);
  bool compileSync(const std::string &cmd);
};
} // namespace livrn
