cmake_minimum_required (VERSION 3.2)
project (REDISBENCH)

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake/module)

find_package(hiredis REQUIRED)
if (NOT HIREDIS_FOUND)
    message(FATAL_ERROR, "hiredis not found.")
endif()

file (GLOB SOURCES src/*.cpp)
file (GLOB HEADERS include/*.hpp)

add_executable (redis_bench ${SOURCES} ${HEADERS})
target_link_libraries (redis_bench ${HIREDIS_LIBS} event)
