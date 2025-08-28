#!/usr/bin/env bash

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}==> Configuring build...${NC}"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

echo -e "${GREEN}==> Building...${NC}"
cmake --build build --target liverun

# Detect platform
OS="$(uname)"
INSTALL_PATH=""
TARGET=""

if [[ "$OS" == "Darwin" ]]; then
    echo -e "${GREEN}Detected macOS.${NC}"
    INSTALL_PATH="/usr/local/bin"
    TARGET="$INSTALL_PATH/liverun"
elif [[ "$OS" == "Linux" ]]; then
    echo -e "${GREEN}Detected Linux.${NC}"
    INSTALL_PATH="/usr/local/bin"
    TARGET="$INSTALL_PATH/liverun"
elif [[ "$OS" =~ "MINGW" || "$OS" =~ "MSYS" || "$OS" =~ "CYGWIN" ]]; then
    echo -e "${GREEN}Detected Windows (Git Bash or MSYS).${NC}"
    INSTALL_PATH="/c/Program Files/liverun"
    mkdir -p "$INSTALL_PATH"
    TARGET="$INSTALL_PATH/liverun.exe"
else
    echo -e "${RED}Unknown OS: $OS. Exiting.${NC}"
    exit 1
fi

# Remove existing binary if present
if [[ -f "$TARGET" ]]; then
    echo -e "${GREEN}Removing existing binary: $TARGET${NC}"
    if [[ "$OS" == "Linux" || "$OS" == "Darwin" ]]; then
        sudo rm -f "$TARGET"
    else
        rm -f "$TARGET"
    fi
fi

# Copy new binary
BIN_PATH="build/liverun"
echo -e "${GREEN}==> Installing new binary to $TARGET...${NC}"
if [[ "$OS" == "Linux" || "$OS" == "Darwin" ]]; then
    sudo cp "$BIN_PATH" "$TARGET"
    sudo chmod +x "$TARGET"
else
    cp "$BIN_PATH.exe" "$TARGET.exe"
fi

echo -e "${GREEN}✅ Installed liverun to: $TARGET${NC}"

# Check PATH
if [[ ":$PATH:" != *":$INSTALL_PATH:"* ]]; then
    echo -e "${RED}⚠️ '$INSTALL_PATH' is not in your PATH.${NC}"
    echo "You can add it with:"
    echo "  export PATH=\"$INSTALL_PATH:\$PATH\""
fi
