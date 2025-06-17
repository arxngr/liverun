#include "core.h"
#include "logger.h"
#include "process/manager.h"
#include "reloader.h"
#include <csignal>
#include <iostream>

namespace livrn {

// Global process manager for signal handling
ProcessManager *g_processManager = nullptr;
livrn::Reloader hotReloader;

void signalHandler(int signal) {
  livrn::Logger::debug("Received signal ", signal, ", cleaning up...");
  if (g_processManager) {
    g_processManager->cleanup();
  }
  exit(signal);
}

void Core::printUsage() {
  std::cerr << "Usage: ./liverun <mode> [args...]\n";
  std::cerr << "Modes:\n";
  std::cerr << "  interpret <interpreter> <script>\n";
  std::cerr << "  compile <binary> <compile_cmd>\n";
  std::cerr << "  command <args1> <args2> [...]\n";
}

void Core::setupSignalHandlers() {
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
}

int Core::run(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage();
    return 1;
  }

  setupSignalHandlers();
  hotReloader.initialize();

  std::string mode = argv[1];

  try {
    if (mode == "interpret") {
      if (argc < 4) {
        livrn::Logger::error(
            "Usage: ./liverun interpret <interpreter> <script>");
        return 1;
      }
      return hotReloader.runInterpretMode(argv[2], argv[3]);

    } else if (mode == "compile") {
      if (argc < 4) {
        livrn::Logger::error("Usage: ./liverun compile <binary> <compile_cmd>");
        return 1;
      }
      return hotReloader.runCompileMode(argv[2], argv[3]);

    } else if (mode == "command") {
      if (argc < 3) {
        livrn::Logger::error("Usage: ./liverun command <args1> <args2> [...]");
        return 1;
      }

      std::vector<std::string> commands;
      for (int i = 2; i < argc; ++i) {
        commands.emplace_back(argv[i]);
      }

      return hotReloader.runCommandMode(commands);

    } else {
      std::cerr << "Unknown mode: " << mode << "\n";
      printUsage();
      return 1;
    }
  } catch (const std::exception &e) {
    livrn::Logger::error("Unhandled exception: ", e.what());
    return 1;
  }
}

} // namespace livrn
