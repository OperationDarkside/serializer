cmake_minimum_required(VERSION 3.0.0)
project(serializer VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O -std=c++2a")
# add_compile_options(-fconcepts)

include(CTest)
enable_testing()

add_executable(serializer testing.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
