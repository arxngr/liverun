
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

pid_t child_pid = -1;
pid_t compile_pid = -1; // Track compilation process
std::unordered_map<std::string, fs::file_time_type> last_modified;
std::unordered_map<std::string, std::filesystem::file_time_type>
    file_timestamps;

const std::unordered_set<std::string> ALLOWED_COMMANDS = {
    "make", "go", "gcc", "g++", "clang", "clang++", "cargo", "npm", "yarn"};

const std::unordered_set<std::string> ALLOWED_EXTENSIONS = {
    ".c",  ".cpp", ".cc", ".cxx", ".h", ".hpp",
    ".go", ".rs",  ".js", ".ts",  ".py"};

const size_t MAX_COMMAND_LENGTH = 1024;
const size_t MAX_PATH_LENGTH = 512;

std::vector<std::string> parse_safe_command(const std::string &cmd) {
  std::vector<std::string> args;
  std::istringstream iss(cmd);
  std::string arg;

  while (iss >> arg) {
    if (arg.length() > 256)
      continue; // Skip overly long args

    // Remove quotes if present
    if (arg.front() == '"' && arg.back() == '"') {
      arg = arg.substr(1, arg.length() - 2);
    }

    args.push_back(arg);
  }

  return args;
}

bool is_safe_path(const std::string &path) {
  try {
    fs::path p(path);
    fs::path canonical = fs::canonical(p.parent_path()) / p.filename();
    fs::path cwd = fs::current_path();

    auto rel = fs::relative(canonical, cwd);
    if (rel.string().substr(0, 2) == "..") {
      return false;
    }

    if (canonical.string().length() > MAX_PATH_LENGTH) {
      return false;
    }

    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool is_safe_command(const std::string &cmd) {
  if (cmd.empty() || cmd.length() > MAX_COMMAND_LENGTH) {
    return false;
  }

  const std::string dangerous_chars = ";|&`$(){}[]<>*?";
  if (cmd.find_first_of(dangerous_chars) != std::string::npos) {
    return false;
  }

  std::istringstream iss(cmd);
  std::string first_word;
  iss >> first_word;

  return ALLOWED_COMMANDS.find(first_word) != ALLOWED_COMMANDS.end();
}
std::vector<std::string> parse_command(const std::string &cmd) {
  std::vector<std::string> args;
  std::istringstream iss(cmd);
  std::string arg;

  while (iss >> arg) {
    // Basic validation for each argument
    if (arg.length() > 256)
      continue; // Skip overly long args

    // Remove quotes if present
    if (arg.front() == '"' && arg.back() == '"') {
      arg = arg.substr(1, arg.length() - 2);
    }

    args.push_back(arg);
  }

  return args;
}

bool is_binary_file(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open())
    return false;

  char buffer[512];
  file.read(buffer, sizeof(buffer));
  std::streamsize bytesRead = file.gcount();

  for (std::streamsize i = 0; i < bytesRead; ++i) {
    unsigned char c = buffer[i];
    if (c < 9 || (c > 13 && c < 32) || c == 127) {
      return true;
    }
  }

  return false;
}

void scan_directory(const std::filesystem::path &dir) {
  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(dir)) {
      if (!entry.is_regular_file())
        continue;

      std::string path_str = entry.path().string();

      if (!is_safe_path(path_str)) {
        std::cout << "[security] Skipping unsafe path: " << path_str
                  << std::endl;
        continue;
      }

      std::string ext = entry.path().extension().string();
      if (ALLOWED_EXTENSIONS.find(ext) == ALLOWED_EXTENSIONS.end()) {
        continue;
      }

      if (!is_binary_file(entry.path())) {
        file_timestamps[path_str] = std::filesystem::last_write_time(entry);
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "[error] Directory scan failed: " << e.what() << std::endl;
  }
}
bool has_any_file_changed() {
  for (auto &[path, old_time] : file_timestamps) {
    if (!fs::exists(path))
      continue; // Skip if file was deleted

    auto current_time = std::filesystem::last_write_time(path);
    if (current_time != old_time) {
      std::cout << "[liverun] File changed: " << path << std::endl;
      file_timestamps[path] = current_time;
      return true;
    }
  }
  return false;
}

void kill_child_group() {
  if (child_pid > 0) {
    std::cout << "[liverun] Stopping application (PID: " << child_pid
              << ")...\n";

    // Validate PID is actually our child
    int status;
    pid_t result = waitpid(child_pid, &status, WNOHANG);
    if (result == -1) {
      child_pid = -1;
      return;
    }

    if (kill(child_pid, SIGTERM) == 0) {
      // Wait for graceful shutdown
      for (int i = 0; i < 30; ++i) {
        result = waitpid(child_pid, &status, WNOHANG);
        if (result == child_pid) {
          std::cout << "[liverun] Application stopped gracefully\n";
          child_pid = -1;
          return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      // Force kill if necessary
      std::cout << "[liverun] Force killing application...\n";
      kill(child_pid, SIGKILL);
      waitpid(child_pid, nullptr, 0);
    }

    child_pid = -1;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void kill_compile_process() {
  if (compile_pid > 0) {
    kill(compile_pid, SIGTERM);
    waitpid(compile_pid, nullptr, 0);
    compile_pid = -1;
  }
}

bool compile(const std::string &cmd) {
  if (!is_safe_command(cmd)) {
    std::cerr << "[security] Unsafe command rejected: " << cmd << std::endl;
    return false;
  }

  std::cout << "[liverun] Compiling: " << cmd << std::endl;

  // Use safer execution
  auto args = parse_safe_command(cmd);
  if (args.empty())
    return false;

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    std::vector<char *> argv;
    for (auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    // Use execvp instead of shell
    execvp(argv[0], argv.data());
    exit(1);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
  }

  return false;
}

// Non-blocking compilation using fork/exec
bool compile_nonblocking(const std::string &cmd) {
  if (!is_safe_command(cmd)) {
    std::cerr << "[security] Unsafe command rejected: " << cmd << std::endl;
    return false;
  }

  std::cout << "[liverun] Starting compilation: " << cmd << std::endl;

  kill_compile_process();

  auto args = parse_safe_command(cmd);
  if (args.empty())
    return false;

  compile_pid = fork();
  if (compile_pid == 0) {
    std::vector<char *> argv;
    for (auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    execvp(argv[0], argv.data());
    exit(1);
  }

  return compile_pid > 0;
}

int check_compile_status() {
  if (compile_pid <= 0)
    return -1; // No compilation running

  int status;
  pid_t result = waitpid(compile_pid, &status, WNOHANG); // Non-blocking wait

  if (result == 0) {
    return 0; // Still running
  } else if (result == compile_pid) {
    compile_pid = -1;
    if (WIFEXITED(status)) {
      return WEXITSTATUS(status) == 0 ? 1 : -2; // 1 = success, -2 = failed
    }
    return -2;
  }

  return -1;
}

void start_interpreter(const std::string &interpreter,
                       const std::string &script) {
  child_pid = fork();
  if (child_pid == 0) {
    // Create new process group so we can kill all child processes
    setpgid(0, 0);
    execlp(interpreter.c_str(), interpreter.c_str(), script.c_str(), nullptr);
    std::cerr << "Failed to start interpreter\n";
    exit(1);
  }
}

void start_binary(const std::string &binary) {
  child_pid = fork();
  if (child_pid == 0) {
    // Create new process group so we can kill all child processes
    setpgid(0, 0);
    execl(binary.c_str(), binary.c_str(), nullptr);
    std::cerr << "Failed to start binary\n";
    exit(1);
  }
}

void start_command(const std::string &cmd) {
  if (!is_safe_command(cmd)) {
    std::cerr << "[security] Unsafe command rejected: " << cmd << std::endl;
    return;
  }

  auto args = parse_safe_command(cmd);
  if (args.empty())
    return;

  child_pid = fork();
  if (child_pid == 0) {
    setpgid(0, 0);

    std::vector<char *> argv;
    for (auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    execvp(argv[0], argv.data());
    std::cerr << "Failed to start command: " << cmd << std::endl;
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: ./liverun <mode> [args...]\n";
    return 1;
  }

  std::string mode = argv[1];
  scan_directory(".");

  if (mode == "interpret") {
    if (argc < 4) {
      std::cerr << "Usage: ./liverun interpret <interpreter> <script>\n";
      return 1;
    }

    std::string interpreter = argv[2];
    std::string script = argv[3];
    last_modified[script] = fs::last_write_time(script);
    start_interpreter(interpreter, script);

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      if (has_any_file_changed()) {
        std::cout << "[liverun] Change detected. Restarting...\n";
        kill_child_group();
        start_interpreter(interpreter, script);
      }
    }
  } else if (mode == "compile") {
    if (argc < 5) {
      std::cerr << "Usage: ./liverun compile <binary> <compile_cmd> <src1> "
                   "[src2...]\n";
      return 1;
    }

    std::string binary = argv[2];
    std::string compile_cmd = argv[3];
    std::vector<std::string> sources;
    for (int i = 4; i < argc; ++i) {
      sources.push_back(argv[i]);
      last_modified[sources.back()] = fs::last_write_time(sources.back());
    }

    if (!compile(compile_cmd)) {
      std::cerr << "[liverun] Initial compilation failed\n";
      return 1;
    }

    start_binary(binary);

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      if (has_any_file_changed()) {
        std::cout << "[liverun] Source change detected\n";
        kill_child_group();
        if (compile(compile_cmd)) {
          start_binary(binary);
        } else {
          std::cerr << "[liverun] Compilation failed\n";
        }
      }
    }
  } else if (mode == "run") {
    if (argc < 4) {
      std::cerr << "Usage: ./liverun run <compile_cmd> <run_cmd>\n";
      return 1;
    }

    std::string compile_cmd = argv[2];
    std::string run_cmd = argv[3];
    bool compilation_in_progress = false;

    // Initial compilation
    if (!compile(compile_cmd)) {
      std::cerr << "[liverun] Initial compilation failed\n";
      return 1;
    }

    // Start the application
    std::cout << "[liverun] Starting application...\n";
    start_command(run_cmd);

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      if (compilation_in_progress) {
        int status = check_compile_status();
        if (status == 1) {
          std::cout << "[liverun] Compilation completed successfully. "
                       "Restarting application...\n";
          kill_child_group(); // Kill the running application and wait for port
                              // release

          // Start the application again
          start_command(run_cmd);
          compilation_in_progress = false;
        } else if (status == -2) {
          std::cerr << "[liverun] Compilation failed. Application continues "
                       "running.\n";
          compilation_in_progress = false;
        }
      }

      // Check for file changes
      if (has_any_file_changed()) {
        std::cout << "[liverun] Source change detected. Compiling...\n";
        if (compilation_in_progress) {
          std::cout << "[liverun] Stopping previous compilation\n";
          kill_compile_process();
        }

        if (compile_nonblocking(compile_cmd)) {
          compilation_in_progress = true;
        } else {
          std::cerr << "[liverun] Failed to start compilation\n";
        }
      }
    }
  } else {
    std::cerr << "Unknown mode: " << mode << "\n";
    std::cerr << "Available modes: interpret, compile, pre, run\n";
    return 1;
  }

  return 0;
}
