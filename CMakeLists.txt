cmake_minimum_required(VERSION 2.4)
project(sr)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Flex)
include(FindGLIB2)
include(CheckedAddSubdir)

add_subdirectory_if_exists(common)
add_subdirectory_if_exists(contrib)
add_subdirectory_if_exists(testbed)
add_subdirectory_if_exists(stemmer2)
add_subdirectory_if_exists(conv2)

