project(stemmer2)

set(EXECUTABLE_OUTPUT_PATH "../bin")

include_directories(sb/include ${GLIB2_INCLUDE_DIR})
add_subdirectory(sb)

set(source main.cpp)
add_flex_files(source tokenizer.l)

add_executable(stemmer2 ${source})
target_link_libraries(stemmer2 sb hunspell-1.2 fl)

