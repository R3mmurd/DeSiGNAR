# An interface library to make the warnings level available to other targets
# This interface taget is set-up through the platform specific script
add_library(designar-warning-interface INTERFACE)

# An interface library to make the target com available to other targets
add_library(designar-compile-option-interface INTERFACE)