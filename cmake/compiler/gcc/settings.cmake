set(GCC_EXPECTED_VERSION 9.0.0)

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_EXPECTED_VERSION)
  message(FATAL_ERROR "GCC: Designar requires version ${GCC_EXPECTED_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else()
  message(STATUS "GCC: Minimum version required is ${GCC_EXPECTED_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - ok!")
endif()

# GCC Warning Options (mirrors cmake/compiler/clang/settings.cmake so both
# compilers get the same coverage; previously this file was empty, meaning a
# GCC build had *no* warnings enabled at all, and CI only ever exercised the
# Clang path).
target_compile_options(designar-warning-interface
  INTERFACE
    -Wall
    -Wextra
    -Wcast-align
    -Wno-sign-compare
    -Wno-write-strings
    -Wno-parentheses
    -Wfloat-equal
    -pedantic
)

# Hardening: detect stack-buffer corruption before a corrupted return
# address/saved registers can be used, and enable glibc's additional
# bounds-checked replacements for common libc calls (memcpy, sprintf, etc).
# _FORTIFY_SOURCE only takes effect at -O1 and above.
target_compile_options(designar-compile-option-interface
  INTERFACE
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)

message(STATUS "GCC: All warnings enabled")
