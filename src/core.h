#pragma once
#include "liverun.h"

namespace livrn {

class Core {
public:
  int run(int argc, char *argv[]);

private:
  void printUsage();
  void setupSignalHandlers();
};

} // namespace livrn
