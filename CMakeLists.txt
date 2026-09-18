cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(src/adventure)
add_subdirectory(src/furelise)
add_subdirectory(src/gamepad)
add_subdirectory(src/mandelbrot)
add_subdirectory(src/nfc)
add_subdirectory(src/paint)
add_subdirectory(src/term)
