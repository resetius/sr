project(testbed)

include_directories(/home/manwe/inst/include)
link_directories(/home/manwe/inst/lib)

add_executable(testbed main.c markov.c)
target_link_libraries(testbed event)

