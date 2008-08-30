project(testbed)

add_executable(testbed main.c markov.c)
target_link_libraries(testbed event)

