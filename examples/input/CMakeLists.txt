cmake_minimum_required(VERSION 3.20)
project(input C ASM)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/hng64kit.cmake")

add_executable(input main.c)
hng64_add_rom(input)
