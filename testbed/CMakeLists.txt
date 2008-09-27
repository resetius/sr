project(testbed)

include_directories(../contrib/libevent)
link_directories(../lib)

add_definitions(-DIDEAL_HASHING)
add_executable(testbed main.c markov.c)

if (NOT CYGWIN)
	set(ext_libs rt)
endif(NOT CYGWIN)

target_link_libraries(testbed pthread event ${ext_libs})

