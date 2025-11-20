cmake_minimum_required(VERSION 3.10)
project(regex_nfa LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Build options
option(BUILD_TESTS "Build small test runner" ON)


include_directories(${CMAKE_SOURCE_DIR}/include)


add_library(regexnfa_lib
src/nfa.cpp
src/regex_parser.cpp
src/thompson.cpp
)


target_include_directories(regexnfa_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)


add_executable(regex_nfa_cli src/main.cpp)
target_link_libraries(regex_nfa_cli PRIVATE regexnfa_lib)


if(BUILD_TESTS)
add_executable(regex_nfa_test src/main.cpp) # same binary contains interactive & tests; args select mode
target_link_libraries(regex_nfa_test PRIVATE regexnfa_lib)
endif()


install(TARGETS regex_nfa_cli RUNTIME DESTINATION bin)
