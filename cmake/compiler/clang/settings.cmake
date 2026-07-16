set(CLANG_EXPECTED_VERSION 14.0.0)

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS CLANG_EXPECTED_VERSION)
  message(FATAL_ERROR "Clang: Designar requires version ${CLANG_EXPECTED_VERSION} to build but found ${CMAKE_CXX_COMPILER_VERSION}")
else()
  message(STATUS "Clang: Minimum version required is ${CLANG_EXPECTED_VERSION}, found ${CMAKE_CXX_COMPILER_VERSION} - ok!")
endif()

# Clang Warning Options
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

message(STATUS "Clang: All warnings enabled")

# Hardening: detect stack-buffer corruption before a corrupted return
# address/saved registers can be used, and enable glibc's additional
# bounds-checked replacements for common libc calls (memcpy, sprintf, etc).
# _FORTIFY_SOURCE only takes effect at -O1 and above.
target_compile_options(designar-compile-option-interface
  INTERFACE
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
)
