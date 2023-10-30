cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(rp6502-sdk)

add_executable(altair)
rp6502_asset(altair 0x10000 src/altair.pal.bin)
rp6502_asset(altair 0x10200 src/altair.dat.bin)
rp6502_executable(altair 0x0200 0x0200
    ${CMAKE_CURRENT_LIST_DIR}/src/altair.hlp.rp6502
    altair.pal.rp6502
    altair.dat.rp6502
)
target_sources(altair PRIVATE
    src/altair.c
)
target_link_libraries(altair PRIVATE
    rp6502
)

add_executable(affine)
rp6502_executable(affine)
target_sources(affine PRIVATE
    src/affine.c
)
target_link_libraries(affine PRIVATE
    rp6502
)

add_executable(mandelbrot)
rp6502_executable(mandelbrot)
target_sources(mandelbrot PRIVATE
    src/mandelbrot.c
)
target_link_libraries(mandelbrot PRIVATE
    rp6502
)

add_executable(mode1)
rp6502_executable(mode1)
target_sources(mode1 PRIVATE
    src/mode1.c
)
target_link_libraries(mode1 PRIVATE
    rp6502
)

add_executable(mode2)
rp6502_executable(mode2)
target_sources(mode2 PRIVATE
    src/mode2.c
)
target_link_libraries(mode2 PRIVATE
    rp6502
)

add_executable(mode3)
rp6502_executable(mode3)
target_sources(mode3 PRIVATE
    src/mode3.c
)
target_link_libraries(mode3 PRIVATE
    rp6502
)

add_executable(paint)
rp6502_executable(paint)
target_sources(paint PRIVATE
    src/paint.c
)
target_link_libraries(paint PRIVATE
    rp6502
)

add_executable(palette)
rp6502_executable(palette)
target_sources(palette PRIVATE
    src/palette.c
)
target_link_libraries(palette PRIVATE
    rp6502
)

add_executable(raspberry)
rp6502_executable(raspberry)
target_sources(raspberry PRIVATE
    src/raspberry.c
)
target_link_libraries(raspberry PRIVATE
    rp6502
)

add_executable(vsync)
rp6502_executable(vsync)
target_sources(vsync PRIVATE
    src/vsync.c
)
target_link_libraries(vsync PRIVATE
    rp6502
)
