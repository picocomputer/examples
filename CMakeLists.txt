cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MY-RP6502-PROJECT C ASM)

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/adventure/troglobit/src/advent.c)
    execute_process(
        COMMAND git submodule update --init src/adventure/troglobit
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/adventure/troglobit/src/advent.c)
    message(FATAL_ERROR "Adventure needs its submodule:\n"
        "  git submodule update --init src/adventure/troglobit")
endif()

add_executable(adventure)
rp6502_asset(adventure help src/adventure/adventure.hlp)
rp6502_asset(adventure /advent1.txt src/adventure/troglobit/src/advent1.txt)
rp6502_asset(adventure /advent2.txt src/adventure/troglobit/src/advent2.txt)
rp6502_asset(adventure /advent3.txt src/adventure/troglobit/src/advent3.txt)
rp6502_asset(adventure /advent4.txt src/adventure/troglobit/src/advent4.txt)
rp6502_executable(adventure DATA default RESET default)
target_include_directories(adventure PRIVATE
    src/adventure
)
target_sources(adventure PRIVATE
    src/adventure/err.c
    src/adventure/troglobit/src/advent.c
    src/adventure/troglobit/src/database.c
    src/adventure/troglobit/src/english.c
    src/adventure/troglobit/src/itverb.c
    src/adventure/troglobit/src/turn.c
    src/adventure/troglobit/src/verb.c
)

add_executable(furelise)
rp6502_asset(furelise help src/furelise/furelise.hlp)
rp6502_executable(furelise DATA default RESET default)
target_sources(furelise PRIVATE
    src/furelise/furelise.c
    src/furelise/ezpsg.c
)

add_executable(gamepad)
rp6502_asset(gamepad help src/gamepad/gamepad.hlp)
rp6502_executable(gamepad DATA default RESET default)
target_sources(gamepad PRIVATE
    src/gamepad/gamepad.c
)

add_executable(mandelbrot)
rp6502_asset(mandelbrot help src/mandelbrot/mandelbrot.hlp)
rp6502_executable(mandelbrot DATA default RESET default)
target_sources(mandelbrot PRIVATE
    src/mandelbrot/mandelbrot.c
)

add_executable(nfc)
rp6502_asset(nfc help src/nfc/nfc.hlp)
rp6502_executable(nfc DATA default RESET default)
target_sources(nfc PRIVATE
    src/nfc/nfc.c
)

add_executable(paint_mou)
rp6502_asset(paint_mou help src/paint_mou/paint_mou.hlp)
rp6502_executable(paint_mou DATA default RESET default)
target_sources(paint_mou PRIVATE
    src/paint_mou/paint_mou.c
)

add_executable(paint_tab)
rp6502_asset(paint_tab help src/paint_tab/paint_tab.hlp)
rp6502_executable(paint_tab DATA default RESET default)
target_sources(paint_tab PRIVATE
    src/paint_tab/paint_tab.c
)

add_executable(term)
rp6502_asset(term help src/term/term.hlp)
rp6502_executable(term DATA default RESET default)
target_sources(term PRIVATE
    src/term/term.c
)
