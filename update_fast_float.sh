#!/bin/bash

# Exit on any error
set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}Fast Float Library Update Script${NC}"
echo -e "${BLUE}=====================================${NC}"

# Create a temporary directory for the build
BUILD_DIR="/tmp/fast_float_build"

# Check for sudo
if [[ $EUID -ne 0 ]]; then
   echo -e "${YELLOW}This script needs to run commands with sudo for installation${NC}"
   echo -e "${YELLOW}You may be prompted for your password${NC}"
fi

# Create and enter build directory
echo -e "\n${GREEN}Creating build directory at ${BUILD_DIR}...${NC}"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Update fast_float
echo -e "\n${GREEN}==== UPDATING FAST FLOAT ====${NC}"
git clone https://github.com/fastfloat/fast_float.git
cd fast_float

# Build and install
mkdir -p build && cd build
cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
make
sudo make install

# Cleanup
cd /tmp
rm -rf "${BUILD_DIR}"

echo -e "\n${GREEN}=====================================${NC}"
echo -e "${GREEN}Fast Float library updated successfully!${NC}"
echo -e "${GREEN}=====================================${NC}"
