cmake_minimum_required(VERSION 4.1)
project(Lognuts)

add_definitions(-DDB_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Lognuts_DB.dat")

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Lognuts
        056_t1ar_prak1.cpp)
