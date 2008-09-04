project(testbed)

include_directories(../contrib/libevent)
link_directories(../lib)

add_definitions(-DIDEAL_HASHING)
add_executable(testbed main.c markov.c)
target_link_libraries(testbed pthread rt event)

