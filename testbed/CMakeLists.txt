project(testbed)

include_directories(../contrib/libevent ../common)
link_directories(../lib)

add_definitions(-DIDEAL_HASHING)
add_executable(testbed main.c markov.c gen_config.cpp)
target_link_libraries(testbed pthread rt common event)

