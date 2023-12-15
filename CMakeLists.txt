cmake_minimum_required(VERSION 3.15)
project(Guu VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wall -Wextra -Werror")
endif()

include(CTest)
enable_testing()

add_executable(
    Guu
        main.cpp
        guu/token.cpp
        guu/lexer.cpp
        guu/ast.cpp
        guu/parser.cpp
        guu/interpreter.cpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)