#!/bin/bash

# Exit on any error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Starting build process for Secure App Server...${NC}"

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p build
fi

# Navigate to build directory
cd build

# Generate build files with CMake
echo -e "${YELLOW}Running CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Executable located at: $(pwd)/secure_app_server${NC}"
    echo -e "${YELLOW}To run the server:${NC}"
    echo -e "${YELLOW}  ./secure_app_server --config=../config/server_config.json --verbose${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Create SSL directory for certificates
if [ ! -d "../ssl" ]; then
    echo -e "${YELLOW}Creating SSL directory for certificates...${NC}"
    mkdir -p ../ssl
fi

# Check if certificate exists
if [ ! -f "../ssl/cert.pem" ] || [ ! -f "../ssl/key.pem" ]; then
    echo -e "${YELLOW}Generating self-signed SSL certificate for development...${NC}"
    openssl req -x509 -newkey rsa:4096 -keyout ../ssl/key.pem -out ../ssl/cert.pem -days 365 -nodes -subj "/CN=localhost"
fi

# Create logs directory if it doesn't exist
if [ ! -d "../logs" ]; then
    echo -e "${YELLOW}Creating logs directory...${NC}"
    mkdir -p ../logs
fi

echo -e "${GREEN}Setup complete!${NC}"
echo -e "${YELLOW}For PostgreSQL setup:${NC}"
echo -e "1. Make sure PostgreSQL server is running"
echo -e "2. Create database: ${GREEN}CREATE DATABASE secure_app;${NC}"
echo -e "3. Create user: ${GREEN}CREATE USER app_user WITH PASSWORD 'change_this_password';${NC}"
echo -e "4. Grant privileges: ${GREEN}GRANT ALL PRIVILEGES ON DATABASE secure_app TO app_user;${NC}"
echo -e "5. Update database settings in config/server_config.json if needed"
