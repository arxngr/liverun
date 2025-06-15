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

int Reloader::runCommandMode(const std::string &compileCmd,
                             const std::string &runCmd) {
  try {
    if (!compiler.compileSync(compileCmd)) {
      std::cerr << "[hotreload] Initial compilation failed\n";
      return 1;
    }

    std::cout << "[hotreload] Starting application...\n";
    if (!processManager.startCommand(runCmd)) {
      std::cerr << "[hotreload] Failed to start application\n";
      return 1;
    }

    while (true) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::POLL_INTERVAL_MS));

      if (monitor.hasAnyFileChanged()) {
        std::cout << "[hotreload] Source change detected. Recompiling and "
                     "restarting...\n";
        processManager.killChild();

        if (compiler.compileSync(compileCmd)) {
          std::cout << "[hotreload] Compilation successful. Restarting "
                       "application...\n";
          processManager.startCommand(runCmd);
        } else {
          std::cerr << "[hotreload] Compilation failed\n";
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[error] Exception in run mode: " << e.what() << std::endl;
    processManager.cleanup();
    return 1;
  }
}

} // namespace livrn
