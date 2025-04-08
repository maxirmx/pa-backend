#!/bin/bash

# Exit on any error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}==============================================${NC}"
echo -e "${BLUE}Facebook C++ Libraries Update Script${NC}"
echo -e "${BLUE}==============================================${NC}"

# Create a temporary directory for the build
BUILD_DIR="~/fb_cpp_build"
CORES=$(nproc)

# Check for sudo
if [[ $EUID -ne 0 ]]; then
   echo -e "${RED}This script needs to run under sudo for installation${NC}"
   exit 1
fi

# Create and enter build directory
echo -e "\n${GREEN}Creating build directory at ${BUILD_DIR}...${NC}"
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Update fast_float
echo -e "\n${GREEN}==== Updating fast float library ====${NC}"
git clone https://github.com/fastfloat/fast_float.git
pushd fast_float

# Build and install
cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON -B build
cmake --build build -j ${CORES}
cmake --install build
popd

echo -e "\n${GREEN}==== Cloning proxygen repository ====${NC}"

git clone https://github.com/facebook/proxygen.git

cd proxygen
git checkout v2025.04.07.00

cd proxygen

echo -e "\n${GREEN}==== Patching proxygen build script ====${NC}"
sed s/\-DCMAKE_INSTALL_PREFIX=\"\$DEPS_DIR\"/\-DCMAKE_INSTALL_PREFIX=\"\$PREFIX\"/ < build.sh > b.sh
chmod +x b.sh

echo -e "\n${GREEN}==== Building proxygen ====${NC}"
./b.sh  --no-tests --prefix /usr/local -j ${CORES}
echo -e "\n${GREEN}==== Installing proxygen ====${NC}"
./install.sh

# Update the shared library cache
echo -e "\n${GREEN}Updating shared library cache...${NC}"
ldconfig

# Cleanup
echo -e "\n${GREEN}Cleaning up...${NC}"
rm -rf "${BUILD_DIR}"

echo -e "\n${GREEN}==============================================${NC}"
echo -e "${GREEN}Facebook C++ Libraries updated successfully!${NC}"
echo -e "${GREEN}==============================================${NC}"
