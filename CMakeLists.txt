cmake_minimum_required(VERSION 3.0)

project(FreeModbus)

add_subdirectory(libfreemodbus)

include_directories(libfreemodbus)

add_executable(test main.c)

target_link_libraries(test freemodbus)


