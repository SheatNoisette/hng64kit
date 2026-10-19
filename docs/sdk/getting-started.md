# Getting started

## Build the toolchain image

From the `hng64kit` folder:

```sh
docker build -t hng64kit:latest .
```

The image holds GCC, newlib, CMake and `hng64kit-mkrom` under `/opt/hng64`.

## Write a program

A program is a `main` function. `crt0` sets up the CPU, the stack and the BSS
before calling it, and `main` should never return:

```c
int main(void)
{
    for (;;) {
    }
}
```

`CMakeLists.txt` includes the SDK rules and calls `hng64_add_rom` on the
executable:

```cmake
cmake_minimum_required(VERSION 3.20)
project(empty C ASM)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/hng64kit.cmake")

add_executable(empty main.c)
hng64_add_rom(empty)
```

`hng64_add_rom` links `crt0` and the linker script and names the output
`empty.elf`. It then runs `hng64kit-mkrom`, which fills in the cart header and
writes the ROM set.

`ASM` must be enabled in `project()` because `crt0` is assembly.

## Build

```sh
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/kit hng64kit:latest -c \
  'cmake -S /kit/examples/empty -B /kit/build/empty -DCMAKE_TOOLCHAIN_FILE=/kit/cmake/hng64.cmake \
   && cmake --build /kit/build/empty'
```

The ROM set lands in `build/empty/roms/roadedge`. MAME has no blank HNG64 defaultsunfortunately, so the cart poses as `roadedge` for now. The program goes in the two PR files, the [font](./text.md) in the four SC files,
and every other chip is an empty file.

## Run in MAME

MAME needs the HNG64 BIOS in a folder named `hng64` somewhere on the rompath.
For legal reasons, of course, we don't ship it.

```sh
mame roadedge -rompath "build/empty/roms;<folder containing hng64>"
```

MAME warns about wrong checksums for every file. Don't worry about it.

To check that the program is running, start MAME with `-debug` and break on
`main`. Get its address with `mips64-elf-nm build/empty/empty.elf` in the
image. Good luck!
