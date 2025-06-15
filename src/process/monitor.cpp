#include "monitor.h"

namespace livrn {

void ProcessMonitor::scanDirectory(const fs::path &dir) {
  try {
    for (const auto &entry : fs::recursive_directory_iterator(dir)) {
      if (!entry.is_regular_file())
        continue;

      std::string pathStr = entry.path().string();
      if (!livrn::Parser::isPathSafe(pathStr)) {
        std::cout << "[security] Skipping unsafe path: " << pathStr
                  << std::endl;
        continue;
      }

      std::string ext = entry.path().extension().string();
      if (Config::ALLOWED_EXTENSIONS.find(ext) ==
          Config::ALLOWED_EXTENSIONS.end()) {
        continue;
      }

      if (!livrn::Parser::isBinaryFile(entry.path())) {
        fileTimestamps[pathStr] = fs::last_write_time(entry);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[error] Directory scan failed: " << e.what() << std::endl;
  }
}

bool ProcessMonitor::hasAnyFileChanged() {
  for (auto &[path, oldTime] : fileTimestamps) {
    if (!fs::exists(path))
      continue;

    auto currentTime = fs::last_write_time(path);
    if (currentTime != oldTime) {
      std::cout << "[livrn] File changed: " << path << std::endl;
      fileTimestamps[path] = currentTime;
      return true;
    }
  }
  return false;
}

} // namespace livrn
