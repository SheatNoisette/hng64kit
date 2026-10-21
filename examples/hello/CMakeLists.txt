cmake_minimum_required(VERSION 3.20)
project(hello C ASM)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/hng64kit.cmake")

add_executable(hello main.c)
hng64_add_rom(hello)
