
# Warning interface in Visual Studio.
# /W4 (rather than the previous /W3) matches the warning coverage level
# targeted for GCC/Clang (-Wall -Wextra) in cmake/compiler/{gcc,clang}.
target_compile_options(designar-warning-interface
  INTERFACE
    /W4)

# disable permissive mode to make msvc more eager to reject code that other compilers don't already accept
# /sdl enables the additional (beyond /GS, which is already on by default)
# "Security Development Lifecycle" checks: stricter runtime checks and extra
# warnings treated as errors for common security-relevant patterns.
target_compile_options(designar-compile-option-interface
  INTERFACE
    /permissive-
    /sdl)
