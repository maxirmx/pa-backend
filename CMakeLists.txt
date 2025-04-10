cmake_minimum_required(VERSION 3.14)  # FetchContent requires CMake 3.14+
project(SecureAppServer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread filesystem regex context)
find_package(OpenSSL REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(fmt 11.0 REQUIRED)


# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${PostgreSQL_INCLUDE_DIRS}
)

# Add source files
file(GLOB_RECURSE SERVER_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Create executable
add_executable(secure_app_server ${SERVER_SRC})


# Link Proxygen libraries
target_link_libraries(secure_app_server
    proxygenhttpserver
    proxygen
    wangle
    fizz
    folly
    mvfst_exception
)

# Link other libraries
target_link_libraries(secure_app_server
    sodium
    z
    zstd
    lzma
    bz2
    lz4
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    glog::glog
    fmt::fmt
    gflags
    double-conversion
    event
    iberty
    snappy
    pthread
    ${PostgreSQL_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# Installation
install(TARGETS secure_app_server DESTINATION bin)

# Copy configuration files to the build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/server_config.json
               ${CMAKE_CURRENT_BINARY_DIR}/server_config.json COPYONLY)

# Testing
enable_testing()
