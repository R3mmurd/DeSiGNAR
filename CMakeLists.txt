cmake_minimum_required(VERSION 3.20)
project(Designar VERSION 2.0.0)

# Set C++ Required version
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED true)

# Use -std=c++XX instead of -std=gnu++XX
set(CMAKE_CXX_EXTENSIONS OFF)

# Append macro directory to CMake Path
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake/macros")

# Import cmake options
include(ConfigureBaseTargets)
include(CheckPlatform)

# Designar Library
add_library(Designar STATIC)
target_include_directories(Designar PUBLIC "${PROJECT_SOURCE_DIR}/include")

# Collect the sources
file(GLOB DESIGNAR_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

target_link_libraries(Designar PUBLIC designar-warning-interface)
target_sources(Designar PUBLIC ${DESIGNAR_SOURCES})

