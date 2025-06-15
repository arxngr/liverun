#pragma once
#include "../liverun.h"
#include "../util/parser.h"

namespace livrn {
class ProcessMonitor {
private:
  std::unordered_map<std::string, fs::file_time_type> fileTimestamps;

public:
  void scanDirectory(const fs::path &dir);
  bool hasAnyFileChanged();
};
} // namespace livrn
