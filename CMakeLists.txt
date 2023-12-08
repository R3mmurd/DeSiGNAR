cmake_minimum_required(VERSION 3.20)
project(Designar VERSION 2.0.0)

# Set C++ Required version
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED true)

# Use -std=c++XX instead of -std=gnu++XX
set(CMAKE_CXX_EXTENSIONS OFF)

# Append macro directory to CMake Path
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake/macros")

# Import cmake options
include(CTest)
include(ConfigureBaseTargets)
include(CheckPlatform)

# Designar Library
file(GLOB DESIGNAR_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_library(Designar STATIC ${DESIGNAR_SOURCES})
target_include_directories(Designar PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(Designar 
  PUBLIC 
    designar-warning-interface
    designar-compile-option-interface
)
install(
  TARGETS Designar
  DESTINATION ${PROJECT_SOURCE_DIR}/lib
)

find_package(Threads REQUIRED)

# Designar Samples
file(GLOB DESIGNAR_SAMPLES_SOURCES "${PROJECT_SOURCE_DIR}/samples/src/*.cpp")
foreach(samplefile ${DESIGNAR_SAMPLES_SOURCES})
  get_filename_component(samplename ${samplefile} NAME_WE)
  add_executable(${samplename} ${samplefile})
  target_link_libraries(${samplename}
    PUBLIC
      Designar
      Threads::Threads
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${samplename}
    DESTINATION ${PROJECT_SOURCE_DIR}/samples/bin
  )
endforeach(samplefile ${DESIGNAR_SAMPLES_SOURCES})

# Designar Tests
file(GLOB DESIGNAR_TESTS_SOURCES "${PROJECT_SOURCE_DIR}/tests/src/*.cpp")
foreach(testfile ${DESIGNAR_TESTS_SOURCES})
  get_filename_component(testname ${testfile} NAME_WE)
  add_executable(${testname} ${testfile})
  target_link_libraries(${testname}
    PUBLIC
      Designar
      Threads::Threads
  )
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${testname}
    DESTINATION ${PROJECT_SOURCE_DIR}/tests/bin
  )
  add_test(
    NAME
      ${testname}
    COMMAND
      ${testname}
  )
endforeach(testfile ${DESIGNAR_TESTS_SOURCES})