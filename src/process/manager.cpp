#include "manager.h"
#include "../logger.h"
#include <fcntl.h>
#include <unistd.h> // for geteuid on Linux/macOS

#ifdef _WIN32
#include <shellapi.h>
#include <windows.h>
#endif
namespace livrn {

void ProcessManager::killProcessGracefully(pid_t &pid,
                                           const std::string &processName) {
  if (pid <= 0)
    return;

  livrn::Logger::warn("Stopping ", processName, " (PID: ", pid, ")...");

  int status;
  pid_t result = waitpid(pid, &status, WNOHANG);
  if (result == -1) {
    pid = -1;
    return;
  }

  if (kill(pid, SIGTERM) == 0) {
    for (int i = 0; i < livrn::Config::GRACEFUL_SHUTDOWN_TIMEOUT_MS / 100;
         ++i) {
      result = waitpid(pid, &status, WNOHANG);
      if (result == pid) {
        livrn::Logger::info(processName, " stopped gracefully");
        pid = -1;
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    livrn::Logger::error("Force killing ", processName);
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
  }

  pid = -1;
}

ProcessManager::~ProcessManager() { cleanup(); }

void ProcessManager::cleanup() noexcept {
  try {
    killChild();
    killCompileProcess();
  } catch (...) {
    // Suppress all exceptions
  }
}

void ProcessManager::killChild() {
  if (childPid > 0) {
    killProcessGracefully(childPid, "application");
    if (childPid == -1) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Config::SHUTDOWN_DELAY_MS));
    }
  }
}

void ProcessManager::killCompileProcess() {
  if (compilePid > 0) {
    kill(compilePid, SIGTERM);
    waitpid(compilePid, nullptr, 0);
    compilePid = -1;
  }
}
bool ProcessManager::startProcess(const std::vector<std::string> &args) {
  if (args.empty())
    return false;

  childPid = fork();
  if (childPid == 0) {
    setpgid(0, 0);

    std::vector<char *> argv;
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    execvp(argv[0], argv.data());
    livrn::Logger::error("Failed to start process");
    exit(1);
  }

  return childPid > 0;
}

bool ProcessManager::startInterpreter(const std::string &interpreter,
                                      const std::string &script) {
  return startProcess({interpreter, script});
}

bool ProcessManager::startBinary(const std::string &binary) {
  return startProcess({binary});
}

bool ProcessManager::startCommand(const std::string &cmd) {
  if (!livrn::Parser::isCommandSafe(cmd)) {
    livrn::Logger::warn("Unsafe command");
    if (!authenticatedUser()) {
      livrn::Logger::error("User authentication failed. Aborting.");
      return false;
    }
  }

  auto args = livrn::Command::parseCommand(cmd);
  return startProcess(args);
}

bool ProcessManager::isChildRunning() const {
  if (childPid <= 0)
    return false;

  int status;
  pid_t result = waitpid(childPid, &status, WNOHANG);
  return result == 0;
}

bool ProcessManager::authenticatedUser() {
#if defined(__unix__) || defined(__APPLE__)
  if (geteuid() == 0) {
    livrn::Logger::info("User is already root, no authentication needed.");
    return true;
  }

  livrn::Logger::info(
      "This command requires admin rights. Please authenticate...");

  // Validate sudo privileges (prompts for password if needed)
  int ret = system("sudo -v");
  if (ret != 0) {
    livrn::Logger::error("Authentication failed. Root privileges required.");
    return false;
  }

  livrn::Logger::info("User authenticated successfully as root.");
  return true;

#elif _WIN32
  BOOL isAdmin = FALSE;
  PSID adminGroup;
  SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
  if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &adminGroup)) {
    livrn::Logger::error("Failed to allocate SID for admin check.");
    return false;
  }

  CheckTokenMembership(NULL, adminGroup, &isAdmin);
  FreeSid(adminGroup);

  if (!isAdmin) {
    livrn::Logger::error("User is not an administrator.");
    return false;
  }

  livrn::Logger::info("User has administrator privileges.");
  return true;

#else
  livrn::Logger::error("Unsupported platform for authentication.");
  return false;
#endif
}

} // namespace livrn
