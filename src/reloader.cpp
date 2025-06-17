#include "reloader.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace livrn {

Reloader::Reloader() : compiler(processManager) {}

Reloader::~Reloader() { processManager.cleanup(); }

void Reloader::initialize() { monitor.scanDirectory("."); }

int Reloader::runInterpretMode(const std::string &interpreter,
                               const std::string &script) {
  try {
    if (!processManager.startInterpreter(interpreter, script)) {
      std::cerr << "[hotreload] Failed to start interpreter\n";
      return 1;
    }

    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      if (monitor.hasAnyFileChanged()) {
        std::cout << "[hotreload] Change detected. Restarting...\n";
        processManager.killChild();
        processManager.startInterpreter(interpreter, script);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[error] Exception in interpret mode: " << e.what()
              << std::endl;
    processManager.cleanup();
    return 1;
  }
}

int Reloader::runCompileMode(const std::string &binary,
                             const std::string &compileCmd) {
  try {
    if (!compiler.compileSync(compileCmd)) {
      std::cerr << "[hotreload] Initial compilation failed\n";
      return 1;
    }

    if (!processManager.startBinary(binary)) {
      std::cerr << "[hotreload] Failed to start binary\n";
      return 1;
    }

    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      if (monitor.hasAnyFileChanged()) {
        std::cout << "[hotreload] Source change detected\n";
        processManager.killChild();

        if (compiler.compileSync(compileCmd)) {
          processManager.startBinary(binary);
        } else {
          std::cerr << "[hotreload] Compilation failed\n";
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[error] Exception in compile mode: " << e.what() << std::endl;
    processManager.cleanup();
    return 1;
  }
}

int Reloader::runCommandMode(const std::vector<std::string> &commands) {
  if (commands.size() < 1) {
    std::cerr << "[hotreload] At least one command (run) is required\n";
    return 1;
  }

  const size_t lastIdx = commands.size() - 1;
  const std::string &runCmd = commands[lastIdx];

  try {
    // Run setup/compile commands
    for (size_t i = 0; i < lastIdx; ++i) {
      std::cout << "[hotreload] Running setup: " << commands[i] << std::endl;
      if (!compiler.compileSync(commands[i])) {
        std::cerr << "[hotreload] Setup command failed: " << commands[i]
                  << "\n";
        return 1;
      }
    }

    std::cout << "[hotreload] Starting application: " << runCmd << std::endl;
    if (!processManager.startCommand(runCmd)) {
      std::cerr << "[hotreload] Failed to start application\n";
      return 1;
    }

    // Hot reload loop
    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      if (monitor.hasAnyFileChanged()) {
        std::cout << "[hotreload] Change detected. Restarting...\n";
        processManager.killChild();

        // Re-run setup commands
        for (size_t i = 0; i < lastIdx; ++i) {
          std::cout << "[hotreload] Re-running setup: " << commands[i]
                    << std::endl;
          if (!compiler.compileSync(commands[i])) {
            std::cerr << "[hotreload] Setup command failed: " << commands[i]
                      << "\n";
            break;
          }
        }

        // Restart application
        std::cout << "[hotreload] Restarting application: " << runCmd
                  << std::endl;
        processManager.startCommand(runCmd);
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "[error] Exception: " << e.what() << std::endl;
    processManager.cleanup();
    return 1;
  }
}

} // namespace livrn
