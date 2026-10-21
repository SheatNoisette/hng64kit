cmake_minimum_required(VERSION 3.20)
project(irq C ASM)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/hng64kit.cmake")

add_executable(irq main.c)
hng64_add_rom(irq)
