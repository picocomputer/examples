cmake_minimum_required(VERSION 3.20)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(tools)

add_executable(affine)
rp6502_executable(affine DATA 0x200 RESET 0x200)
target_sources(affine PRIVATE
    src/affine.c
)

add_executable(altair1)
rp6502_asset(altair1 help src/altair.hlp)
rp6502_asset(altair1 0x10000 src/altair.pal.bin)
rp6502_asset(altair1 0x10200 src/altair.dat.bin)
rp6502_executable(altair1 DATA 0x200 RESET 0x200)
target_sources(altair1 PRIVATE
    src/altair1.c
)

add_executable(altair2)
rp6502_asset(altair2 help src/altair.hlp)
rp6502_asset(altair2 altair.pal src/altair.pal.bin)
rp6502_asset(altair2 altair.dat src/altair.dat.bin)
rp6502_executable(altair2 DATA 0x200 RESET 0x200)
target_sources(altair2 PRIVATE
    src/altair2.c
)

add_executable(attr)
rp6502_executable(attr DATA 0x200 RESET 0x200)
target_sources(attr PRIVATE
    src/attr.c
)

add_executable(bench)
rp6502_executable(bench DATA 0x200 RESET 0x200)
target_sources(bench PRIVATE
    src/bench.c
)

add_executable(dir)
rp6502_executable(dir DATA 0x200 RESET 0x200)
target_sources(dir PRIVATE
    src/dir.c
)

add_executable(exec)
rp6502_executable(exec DATA 0x200 RESET 0x200)
target_sources(exec PRIVATE
    src/exec.c
)

add_executable(exec2)
rp6502_executable(exec2 DATA 0x200 RESET 0x200)
target_sources(exec2 PRIVATE
    src/exec2.c
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

add_executable(mandelbrot)
rp6502_executable(mandelbrot DATA 0x200 RESET 0x200)
target_sources(mandelbrot PRIVATE
    src/mandelbrot.c
)

add_executable(mathcop)
rp6502_executable(mathcop DATA 0x200 RESET 0x200)
target_sources(mathcop PRIVATE
    src/mathcop.c
)

add_executable(3dcube)
rp6502_executable(3dcube DATA 0x200 RESET 0x200)
target_sources(3dcube PRIVATE
    src/3dcube.c
)

add_executable(3dcubeMC)
rp6502_executable(3dcubeMC DATA 0x200 RESET 0x200)
target_sources(3dcubeMC PRIVATE
    src/3dcubeMC.c
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

add_executable(mode5)
rp6502_executable(mode5 DATA 0x200 RESET 0x200)
target_sources(mode5 PRIVATE
    src/mode5.c
)

add_executable(nfc)
rp6502_asset(nfc help src/nfc.hlp)
rp6502_executable(nfc DATA 0x200 RESET 0x200)
target_sources(nfc PRIVATE
    src/nfc.c
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
