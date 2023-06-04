
# Warning interface in Visual Studio
target_compile_options(designar-warning-interface
  INTERFACE
    /W3)

# disable permissive mode to make msvc more eager to reject code that other compilers don't already accept
target_compile_options(designar-compile-option-interface
  INTERFACE
    /permissive-)
