cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MY-RP6502-PROJECT C ASM)

add_executable(furelise)
rp6502_asset(furelise help src/furelise.hlp)
rp6502_executable(furelise DATA default RESET default)
target_sources(furelise PRIVATE
    src/furelise.c
    src/ezpsg.c
)

add_executable(gamepad)
rp6502_asset(gamepad help src/gamepad.hlp)
rp6502_executable(gamepad DATA default RESET default)
target_sources(gamepad PRIVATE
    src/gamepad.c
)

add_executable(mandelbrot)
rp6502_asset(mandelbrot help src/mandelbrot.hlp)
rp6502_executable(mandelbrot DATA default RESET default)
target_sources(mandelbrot PRIVATE
    src/mandelbrot.c
)

add_executable(nfc)
rp6502_asset(nfc help src/nfc.hlp)
rp6502_executable(nfc DATA default RESET default)
target_sources(nfc PRIVATE
    src/nfc.c
)

add_executable(paint_mou)
rp6502_asset(paint_mou help src/paint_mou.hlp)
rp6502_executable(paint_mou DATA default RESET default)
target_sources(paint_mou PRIVATE
    src/paint_mou.c
)

add_executable(paint_tab)
rp6502_asset(paint_tab help src/paint_tab.hlp)
rp6502_executable(paint_tab DATA default RESET default)
target_sources(paint_tab PRIVATE
    src/paint_tab.c
)

add_executable(term)
rp6502_asset(term help src/term.hlp)
rp6502_executable(term DATA default RESET default)
target_sources(term PRIVATE
    src/term.c
)
