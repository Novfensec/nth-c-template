cmake_minimum_required(VERSION 3.16)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(nth_kernel C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include src)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

add_executable(kernel.elf ${SOURCES})

target_compile_options(kernel.elf PRIVATE
    -ffreestanding
    -fno-stack-protector
    -fno-stack-check
    -fno-lto
    -fno-pie
    -no-pie
    -m64
    -march=x86-64
    -mno-red-zone
    -mno-mmx
    -mno-sse
    -mno-sse2
    -Wall
    -Wextra
)

target_link_options(kernel.elf PRIVATE
    -nostdlib
    -static
    -T "${LINKER_SCRIPT}"
    -Wl,-z,max-page-size=0x1000
)

set_target_properties(kernel.elf PROPERTIES
    LINK_DEPENDS "${LINKER_SCRIPT}"
)
