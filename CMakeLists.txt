cmake_minimum_required(VERSION 3.20)
project(Designar VERSION 3.0.1)

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

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS
    "CMAKE_BUILD_TYPE is not set (defaulting to an unoptimized, assertions-on "
    "build). Pass -DCMAKE_BUILD_TYPE=Release for an optimized build, or "
    "RelWithDebInfo/MinSizeRel for one with -DNDEBUG (which disables "
    "assert()). Note: this project's own tests use assert() as their check "
    "mechanism, so building them (DESIGNAR_BUILD_TESTS, ON by default) with "
    "NDEBUG defined leaves several now-genuinely-unused local variables "
    "behind, which fails under -DDESIGNAR_WARNINGS_AS_ERRORS=ON; pass "
    "-DDESIGNAR_BUILD_TESTS=OFF alongside an NDEBUG build type if you only "
    "want the library.")
endif()

# A handful of real invariants in this codebase used to be guarded only by
# assert(), which compiles to nothing under -DNDEBUG (i.e. in most Release
# configurations), silently turning a caught, diagnosable error into
# undefined behavior. Those have since been converted to thrown exceptions,
# but -Werror is what actually keeps warnings that would have caught issues
# like this (e.g. -Wfloat-equal) from being silently ignored. Off by default
# so it doesn't break local development builds against a newer/older
# compiler than CI uses; CI turns it on explicitly.
option(DESIGNAR_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(DESIGNAR_WARNINGS_AS_ERRORS)
  if(MSVC)
    target_compile_options(designar-warning-interface INTERFACE /WX)
  else()
    target_compile_options(designar-warning-interface INTERFACE -Werror)
  endif()
endif()

# Sanitizers catch exactly the class of bug this library has repeatedly
# turned out to have (dangling references, out-of-bounds access, signed
# integer overflow, data races) far faster than staring at code: several
# fixes made during this project's ongoing cleanup were only found by
# hand, the slow way, and would have been caught in seconds under ASan/
# UBSan/TSan. Off by default (they add real runtime overhead and, for
# TSan, are mutually exclusive with the others), turned on in dedicated CI
# jobs instead of the default build.
#
# ASan and UBSan can be combined in one build; TSan cannot be combined
# with either (they instrument memory access differently and the runtimes
# conflict), so it gets its own option.
option(DESIGNAR_SANITIZE_ADDRESS_UB "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(DESIGNAR_SANITIZE_THREAD "Build with ThreadSanitizer" OFF)

if(DESIGNAR_SANITIZE_ADDRESS_UB AND DESIGNAR_SANITIZE_THREAD)
  message(FATAL_ERROR "DESIGNAR_SANITIZE_ADDRESS_UB and DESIGNAR_SANITIZE_THREAD "
                       "cannot both be enabled: ASan/UBSan and TSan instrument "
                       "memory access incompatibly.")
endif()

if(DESIGNAR_SANITIZE_ADDRESS_UB)
  target_compile_options(designar-compile-option-interface
    INTERFACE -fsanitize=address,undefined -fno-sanitize-recover=all -g)
  target_link_options(designar-compile-option-interface
    INTERFACE -fsanitize=address,undefined)
endif()

if(DESIGNAR_SANITIZE_THREAD)
  target_compile_options(designar-compile-option-interface
    INTERFACE -fsanitize=thread -g)
  target_link_options(designar-compile-option-interface
    INTERFACE -fsanitize=thread)
endif()

# GNUInstallDirs must be included before it is referenced below (it
# defines CMAKE_INSTALL_INCLUDEDIR/LIBDIR/BINDIR); target_include_directories
# expands ${CMAKE_INSTALL_INCLUDEDIR} immediately; it is not deferred like
# the $<...> generator expression wrapping it, so calling this after the
# target_include_directories() call below would silently bake in an empty
# install-interface include path.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Designar Library
file(GLOB DESIGNAR_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_library(Designar STATIC ${DESIGNAR_SOURCES})
add_library(Designar::Designar ALIAS Designar)
target_include_directories(Designar PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(Designar
  PUBLIC
    designar-warning-interface
    designar-compile-option-interface
)

# Convenience copy for this repo's own local dev workflow (samples/tests
# already reference ../lib relative to their own build), kept alongside
# (not instead of) the standard-layout install below.
install(
  TARGETS Designar
  DESTINATION ${PROJECT_SOURCE_DIR}/lib
)

# Standard-layout install + CMake package config, so downstream projects
# can do `find_package(Designar)` + `target_link_libraries(... Designar::Designar)`
# instead of copying this repo's headers around by hand.

install(
  TARGETS Designar designar-warning-interface designar-compile-option-interface
  EXPORT DesignarTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

install(
  EXPORT DesignarTargets
  FILE DesignarTargets.cmake
  NAMESPACE Designar::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Designar
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesignarConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/DesignarConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Designar
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/DesignarConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DesignarConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DesignarConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Designar
)

find_package(Threads REQUIRED)

# Consumers that only want the library (e.g. a vcpkg/Conan package build, or
# a downstream project's own `add_subdirectory(Designar)`) have no use for
# this repo's own samples/tests and their install() rules under
# ${PROJECT_SOURCE_DIR}; both default ON so the plain in-tree dev workflow
# (configure, build, ctest) is unaffected.
option(DESIGNAR_BUILD_SAMPLES "Build the Designar samples" ON)
option(DESIGNAR_BUILD_TESTS "Build the Designar tests" ON)

# Designar Samples
if(DESIGNAR_BUILD_SAMPLES)
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
endif()

# Designar Tests
if(DESIGNAR_BUILD_TESTS)
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
endif()

# Source packaging (`cpack --config CPackSourceConfig.cmake`, or
# `make package_source` / `cmake --build . --target package_source`):
# a release tarball should contain exactly what a consumer needs to build
# and link against the library (include/, src/, the CMake machinery, and
# LICENSE/README), not this repo's own test suite, samples, Doxygen
# config, or packaging-tooling recipes — those stay in version control
# for contributors, but have no reason to ship in a release archive.
set(CPACK_PACKAGE_NAME "Designar")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Alejandro Mujica")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}: a teaching-oriented C++ data structures and algorithms library")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "designar-${PROJECT_VERSION}-src")
set(CPACK_SOURCE_IGNORE_FILES
  "/\\\\.git/"
  "/\\\\.github/"
  "/build/"
  "/doc/"
  "/en-doc/"
  "/es-doc/"
  "/tests/"
  "/samples/"
  "/vcpkg/"
  "/obj/"
  "/lib/"
  "Doxyfile"
  "conanfile\\\\.py"
  "\\\\.dockerignore"
  "Dockerfile"
  "\\\\.gitignore"
  "run_tests\\\\.sh"
  "MODERNIZATION"
)
include(CPack)
