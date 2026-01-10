cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(GD32F3_APP "USART/printf" CACHE STRING "The app to compile.")
set(CMAKE_C_STANDARD 99)

set(CORE_LIB_DIR ${CMAKE_SOURCE_DIR}/azure_rtos)

# Set the toolchain if not defined
set(CMAKE_TOOLCHAIN_FILE "cmake/arm-gcc-cortex-m4.cmake")

include(cmake/utilities.cmake)

# Define the Project
project(gd32f3-eval_main C ASM)

add_subdirectory(lib)
add_subdirectory("app/${GD32F3_APP}")