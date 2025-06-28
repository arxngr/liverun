#include "reloader.h"
#include "logger.h"
#include <chrono>
#include <cstdlib>
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
      livrn::Logger::warn("Failed to start interpreter");
      return 1;
    }

    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      if (monitor.hasAnyFileChanged()) {
        livrn::Logger::info("Change detected. Restarting...");
        processManager.killChild();
        processManager.startInterpreter(interpreter, script);
      }
    }
  } catch (const std::exception &e) {
    livrn::Logger::error("Expection in interpreter mode: ", e.what());
    processManager.cleanup();
    return 1;
  }
}

int Reloader::runCompileMode(const std::string &binary,
                             const std::string &compileCmd) {
  try {
    if (!compiler.compileSync(compileCmd)) {
      livrn::Logger::error("Initial compilation failed");
      return 1;
    }

    if (!processManager.startBinary(binary)) {
      livrn::Logger::error("Failed to start binary");
      return 1;
    }

    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      if (monitor.hasAnyFileChanged()) {
        livrn::Logger::info("Source change detected");
        processManager.killChild();

        if (compiler.compileSync(compileCmd)) {
          processManager.startBinary(binary);
        } else {
          livrn::Logger::error("Compilation failed");
        }
      }
    }
  } catch (const std::exception &e) {
    livrn::Logger::error("Exception in compile mode: ", e.what());
    processManager.cleanup();
    return 1;
  }
}

int Reloader::runCommandMode(const std::vector<std::string> &commands) {
  if (commands.size() < 1) {
    livrn::Logger::error("At least one command (run) is required");
    return 1;
  }

  const size_t lastIdx = commands.size() - 1;
  const std::string &runCmd = commands[lastIdx];

  try {
    for (size_t i = 0; i < lastIdx; ++i) {
      livrn::Logger::info("Running setup: ", commands[i]);
      if (!compiler.compileSync(commands[i])) {
        livrn::Logger::error("Setup command failed: ", commands[i]);
        return 1;
      }
    }

    livrn::Logger::info("Starting application... ", runCmd);
    if (!processManager.startCommand(runCmd)) {
      livrn::Logger::error("Failed to start application");
      return 1;
    }

    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      bool failCompile = false;
      if (monitor.hasAnyFileChanged()) {
        livrn::Logger::info("Change detected. Restarting...");
        processManager.killChild();

        for (size_t i = 0; i < lastIdx; ++i) {
          if (!compiler.compileSync(commands[i])) {
            livrn::Logger::error("Setup command failed: ", commands[i]);
            failCompile = true;
            break;
          }
        }

        if (failCompile)
          continue;

        processManager.startCommand(runCmd);
      }
    }
  } catch (const std::exception &e) {
    livrn::Logger::error("Exception in custom mode: ", e.what());
    processManager.cleanup();
    return 1;
  }
}

} // namespace livrn
