#!/usr/bin/env bash

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}==> Uninstalling liverun...${NC}"

# Detect OS and path
OS="$(uname)"
INSTALL_PATH=""

if [[ "$OS" == "Darwin" || "$OS" == "Linux" ]]; then
    INSTALL_PATH="/usr/local/bin/liverun"
elif [[ "$OS" =~ "MINGW" || "$OS" =~ "MSYS" || "$OS" =~ "CYGWIN" ]]; then
    INSTALL_PATH="/c/Program Files/liverun/liverun.exe"
else
    echo -e "${RED}Unknown OS: $OS. Exiting.${NC}"
    exit 1
fi

# Remove binary
if [[ -f "$INSTALL_PATH" ]]; then
    echo -e "${GREEN}Removing: $INSTALL_PATH${NC}"
    if [[ "$OS" == "Linux" || "$OS" == "Darwin" ]]; then
        sudo rm "$INSTALL_PATH"
    else
        rm "$INSTALL_PATH"
    fi
    echo -e "${GREEN}✅ liverun has been removed.${NC}"
else
    echo -e "${RED}⚠️ liverun is not installed at $INSTALL_PATH${NC}"
fi
