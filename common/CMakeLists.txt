project(common)

set(LIBRARY_OUTPUT_PATH "../lib")
set(EXECUTABLE_OUTPUT_PATH "../bin")

add_library(common my_config.cpp my_signal.c)

