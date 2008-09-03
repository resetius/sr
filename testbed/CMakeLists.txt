project(testbed)

include_directories(/home/aozeritsky/inst/include)
link_directories(/home/aozeritsky/inst/lib)

add_definitions(-DIDEAL_HASHING)
add_executable(testbed main.c markov.c)
target_link_libraries(testbed event)

