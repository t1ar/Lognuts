cmake_minimum_required(VERSION 4.1)
project(Lognuts)

add_definitions(-DDB_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Lognuts_DB.dat")
# Memaksa file aplikasi (.exe atau binary) ke folder root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# Menghapus cache agar CLion mendeteksi perubahan lokasi
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Lognuts
        056_t1ar_prak1.cpp)
