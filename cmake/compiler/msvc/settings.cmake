
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

# MSVC's Debug configuration defaults to _ITERATOR_DEBUG_LEVEL=2, which
# wraps every standard-library-facing iterator in an extra debug-checking
# layer. This library's iterators are hand-written (not MSVC STL's own),
# and this project's test suite relies on assert() (only active without
# NDEBUG, i.e. only in Debug) as its actual check mechanism — so Debug is
# not optional the way it might be on GCC/Clang. Disabling the debug
# iterator level keeps assert() active while sidestepping MSVC STL's
# checked-iterator wrapper machinery, which this library's custom
# iterators are not written against.
target_compile_definitions(designar-compile-option-interface
  INTERFACE
    _ITERATOR_DEBUG_LEVEL=0)
