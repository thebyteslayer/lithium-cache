cmake_minimum_required(VERSION 3.20)
project(lithium-cache-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler to clang
set(CMAKE_CXX_COMPILER "clang++")

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(Threads REQUIRED)

# Check for nlohmann/json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/cache.cpp
    src/config.cpp
    src/token.cpp
    src/apiHandler.cpp
    src/fileLogger.cpp
    src/service.cpp
)

# Header files
set(HEADERS
    src/cache.hpp
    src/config.hpp
    src/token.hpp
    src/apiHandler.hpp
    src/fileLogger.hpp
    src/service.hpp
)

# Create executable
add_executable(lithium-cache-service ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(lithium-cache-service 
    PRIVATE 
    Threads::Threads
    nlohmann_json::nlohmann_json
)

# Installation
install(TARGETS lithium-cache-service
    RUNTIME DESTINATION bin
)
