cmake_minimum_required(VERSION 3.10)
project(regex_nfa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build test executables" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# ---- Library (core NFA + Regex + Thompson + DFA) ----
add_library(regexnfa_lib
    src/nfa.cpp
    src/regex_parser.cpp
    src/thompson.cpp
    src/dfa.cpp
    src/nfa_to_dfa.cpp
)

target_include_directories(regexnfa_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# ---- CLI executable (normal usage) ----
add_executable(regex_nfa_cli src/main.cpp)
target_link_libraries(regex_nfa_cli PRIVATE regexnfa_lib)

# ---- Test executables ----
if(BUILD_TESTS)
  add_executable(regex_nfa_test src/test_main.cpp)
  target_link_libraries(regex_nfa_test PRIVATE regexnfa_lib)

  add_executable(NFA_TEST src/NFA_test_main.cpp)
  target_link_libraries(NFA_TEST PRIVATE regexnfa_lib)

  # ---- NEW: DFA test executable ----
  add_executable(DFA_TEST src/DFA_test_main.cpp)
  target_link_libraries(DFA_TEST PRIVATE regexnfa_lib)
endif()

install(TARGETS regex_nfa_cli RUNTIME DESTINATION bin)
