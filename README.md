# 🔥 liverun

A fast, lightweight hot reload tool for development workflows. liverun automatically monitors your source files and restarts your application when changes are detected, supporting multiple programming languages and build systems.

## ✨ Features

- **Multi-language support**: Works with interpreted languages (Python, Node.js, etc.) and compiled languages (C++, Go, Rust, etc.)
- **Multiple execution modes**: 
  - Interpreter mode for scripts
  - Compile mode for binaries
  - Command mode for custom build/run workflows
- **Cross-platform**: Works on Linux, macOS, and other Unix-like systems

## 🚀 Installation

### Build from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/liverun.git
cd liverun

# Build with CMake
mkdir build && cd build
cmake ..
make

# Install (optional)
chmod +x scripts/*.sh
./scripts/install.sh
```

### Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.16+
- POSIX-compliant system (Linux, macOS, etc.)

## 📖 Usage

live operates in three different modes depending on your development workflow:

### 1. Interpreter Mode

For interpreted languages like Python, Node.js, Ruby, etc.:

```bash
liverun interpret python script.py
liverun interpret node app.js
liverun interpret ruby server.rb
```
Note: Ensure this is under the folder structure, if we put the file on any folder structure let say at root it will watch all of them

### 2. Compile Mode

For compiled languages where you want to rebuild and restart a binary:

```bash
liverun compile ./myapp "g++ -o myapp main.cpp"
liverun compile ./server "go build -o server ."
```

### 3. Command Mode

For complex build systems with separate compile and run commands:

```bash
liverun command "make" "./myapp"
liverun command "cargo build" "cargo run"
liverun command "npm run build" "npm start"
```

---



> ⚠️  **Note**
>

> Make sure to organize your files within a specific folder structure.
> If your target file is placed directly in the root directory, `liverun` will monitor **all folders** at the root level, which may lead to unnecessary reloads.



---


## 💡 Examples

### 🐍 Python Development
```bash
# Monitor and restart Python script
liverun interpret python app.py

# Monitor specific directory
cd my-python-project
liverun interpret python src/main.py
```

### ⚡ C++ Development
```bash
# Simple C++ project
liverun compile ./app "g++ -std=c++17 -o app main.cpp"

# With CMake
liverun command "cmake --build build" "./build/myapp"
```

### 🟢 Node.js Development
```bash
# Direct Node.js execution
liverun interpret node server.js

# With build step
liverun command "npm run build" "npm start"
```

### 🔵 Go Development
```bash
# Direct Go run
liverun interpret go "run main.go"

# Build and run binary
liverun compile ./app "go build -o app ."

# Build and run via makefile
liverun command "make" "make run"
```


## 🛠️ Development

### Building Tests

```bash
# Build with tests
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
make
make liverun_tests
```

### Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes with tests
4. Ensure code follows the project style
5. Submit a pull request

### Code Style

- Use C++17 features where appropriate
- Follow RAII principles
- Use meaningful variable and function names
- Include proper error handling
- Add comments for complex logic

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📝 Changelog

### v1.0.0
- Initial release
- Support for interpreter, compile, and command modes
- Cross-platform file monitoring
- Safe command execution
- Process lifecycle management

## 🤝 Contributing

Contributions are welcome!

## 💬 Support

- **Issues**: [GitHub Issues](https://github.com/arngr/liverun/issues)
- **Discussions**: [GitHub Discussions](https://github.com/arngr/liverun/discussions)
- **Documentation**: [Wiki](https://github.com/arngr/liverun/wiki)
