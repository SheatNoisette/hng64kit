cmake_minimum_required(VERSION 3.20)
project(pong C ASM)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/hng64kit.cmake")

add_executable(pong main.c)

set(HNG64KIT_SPRITES
    "${CMAKE_CURRENT_SOURCE_DIR}/paddle.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/ball.png"
)
hng64_add_rom(pong)
