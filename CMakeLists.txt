cmake_minimum_required(VERSION 2.4)
project(sr)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Flex)
include(FindGLIB2)

add_subdirectory(common)
add_subdirectory(contrib)
add_subdirectory(testbed)
add_subdirectory(stemmer2)
add_subdirectory(conv2)

