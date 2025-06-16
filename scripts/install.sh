#!/usr/bin/env bash

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}==> Configuring build...${NC}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

echo -e "${GREEN}==> Building...${NC}"
cmake --build build --target liverun

# Detect platform
OS="$(uname)"
INSTALL_PATH=""

if [[ "$OS" == "Darwin" ]]; then
    echo -e "${GREEN}Detected macOS.${NC}"
    INSTALL_PATH="/usr/local/bin"
elif [[ "$OS" == "Linux" ]]; then
    echo -e "${GREEN}Detected Linux.${NC}"
    INSTALL_PATH="/usr/local/bin"
elif [[ "$OS" =~ "MINGW" || "$OS" =~ "MSYS" || "$OS" =~ "CYGWIN" ]]; then
    echo -e "${GREEN}Detected Windows (Git Bash or MSYS).${NC}"
    INSTALL_PATH="/c/Program Files/liverun"
    mkdir -p "$INSTALL_PATH"
else
    echo -e "${RED}Unknown OS: $OS. Exiting.${NC}"
    exit 1
fi

# Copy binary to target install path
BIN_PATH="build/liverun"
TARGET="$INSTALL_PATH/liverun"

echo -e "${GREEN}==> Copying binary to $TARGET...${NC}"
if [[ "$OS" == "Linux" || "$OS" == "Darwin" ]]; then
    sudo cp "$BIN_PATH" "$TARGET"
    sudo chmod +x "$TARGET"
else
    cp "$BIN_PATH.exe" "$TARGET.exe"
fi

echo -e "${GREEN}✅ Installed liverun to: $TARGET${NC}"

if [[ ":$PATH:" != *":$INSTALL_PATH:"* ]]; then
    echo -e "${RED}⚠️ '$INSTALL_PATH' is not in your PATH.${NC}"
    echo "You can add it with:"
    echo "  export PATH=\"$INSTALL_PATH:\$PATH\""
fi
