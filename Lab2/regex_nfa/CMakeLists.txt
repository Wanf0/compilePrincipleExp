
cmake_minimum_required(VERSION 3.10)
project(regex_nfa LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build test executable
option(BUILD_TESTS "Build small test runner" ON)

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library source files
add_library(regexnfa_lib
    src/nfa.cpp
    src/regex_parser.cpp
    src/thompson.cpp
)

target_include_directories(regexnfa_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Main CLI executable
add_executable(regex_nfa_cli src/main.cpp)
target_link_libraries(regex_nfa_cli PRIVATE regexnfa_lib)

# Optional test executable
if(BUILD_TESTS)
    add_executable(regex_nfa_test src/test_main.cpp)
    target_link_libraries(regex_nfa_test PRIVATE regexnfa_lib)
endif()

# Install
install(TARGETS regex_nfa_cli RUNTIME DESTINATION bin)
