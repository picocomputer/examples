cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(tools)

add_executable(affine)
rp6502_executable(affine DATA 0x200 RESET 0x200)
target_sources(affine PRIVATE
    src/affine.c
)

add_executable(altair)
rp6502_asset(altair 0x10000 src/altair.pal.bin)
rp6502_asset(altair 0x10200 src/altair.dat.bin)
rp6502_executable(altair DATA 0x200 RESET 0x200
    ${CMAKE_CURRENT_LIST_DIR}/src/altair.hlp.rp6502
    altair.pal.bin.rp6502
    altair.dat.bin.rp6502
)
target_sources(altair PRIVATE
    src/altair.c
)

add_executable(dir)
rp6502_executable(dir DATA 0x200 RESET 0x200)
target_sources(dir PRIVATE
    src/dir.c
)

add_executable(furelise)
rp6502_executable(furelise DATA 0x200 RESET 0x200)
target_sources(furelise PRIVATE
    src/furelise.c
    src/ezpsg.c
)

add_executable(gamepad)
rp6502_executable(gamepad DATA 0x200 RESET 0x200)
target_sources(gamepad PRIVATE
    src/gamepad.c
)

add_executable(gets)
rp6502_executable(gets DATA 0x200 RESET 0x200)
target_sources(gets PRIVATE
    src/gets.c
)

add_executable(mandelbrot)
rp6502_executable(mandelbrot DATA 0x200 RESET 0x200)
target_sources(mandelbrot PRIVATE
    src/mandelbrot.c
)

add_executable(mode1)
rp6502_executable(mode1 DATA 0x200 RESET 0x200)
target_sources(mode1 PRIVATE
    src/mode1.c
)

add_executable(mode2)
rp6502_executable(mode2 DATA 0x200 RESET 0x200)
target_sources(mode2 PRIVATE
    src/mode2.c
)

add_executable(mode3)
rp6502_executable(mode3 DATA 0x200 RESET 0x200)
target_sources(mode3 PRIVATE
    src/mode3.c
)

add_executable(paint)
rp6502_executable(paint DATA 0x200 RESET 0x200)
target_sources(paint PRIVATE
    src/paint.c
)

add_executable(palette)
rp6502_executable(palette DATA 0x200 RESET 0x200)
target_sources(palette PRIVATE
    src/palette.c
)

add_executable(poprock)
rp6502_executable(poprock DATA 0x200 RESET 0x200)
target_sources(poprock PRIVATE
    src/poprock.c
    src/ezpsg.c
)

add_executable(raspberry)
rp6502_executable(raspberry DATA 0x200 RESET 0x200)
target_sources(raspberry PRIVATE
    src/raspberry.c
)

add_executable(rtc)
rp6502_executable(rtc DATA 0x200 RESET 0x200)
target_sources(rtc PRIVATE
    src/rtc.c
)

add_executable(term)
rp6502_executable(term DATA 0x200 RESET 0x200)
target_sources(term PRIVATE
    src/term.c
)

add_executable(vsync)
rp6502_executable(vsync DATA 0x200 RESET 0x200)
target_sources(vsync PRIVATE
    src/vsync.c
)
