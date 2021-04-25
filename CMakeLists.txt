cmake_minimum_required(VERSION 3.0)

project(FreeModbus)

add_compile_options(-g)

add_subdirectory(libfreemodbus)
include_directories(libfreemodbus)

add_subdirectory(libsimulation)
include_directories(libsimulation)

add_executable(mytest main.c)

set(CMAKE_C_FLAGS -pthread)

target_link_libraries(mytest freemodbus simulation)


