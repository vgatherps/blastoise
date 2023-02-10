cmake_minimum_required(VERSION 3.13.0)
project(blastoise VERSION 0.1.0)
cmake_policy(SET CMP0076 NEW)
set(CMAKE_CXX_STANDARD 20)

include(CTest)
enable_testing()

add_library(blastoise "")

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

include_directories(include)
add_subdirectory(src)
add_subdirectory(test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
