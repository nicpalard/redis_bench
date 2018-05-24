cmake_minimum_required (VERSION 3.2)
project (REDISBENCH)

set (CMAKE_CXX_STANDARD 11)

file (GLOB SOURCES src/*.cpp)
file (GLOB HEADERS include/*.hpp)

add_executable (redis_bench ${SOURCES} ${HEADERS})
target_link_libraries (redis_bench hiredis event)
