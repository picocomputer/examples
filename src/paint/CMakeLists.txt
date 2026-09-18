find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/logo.bin
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/png2picture.py
        ${CMAKE_CURRENT_SOURCE_DIR}/logo.png
        ${CMAKE_CURRENT_BINARY_DIR}/logo.bin
    DEPENDS png2picture.py logo.png
    VERBATIM
)

add_executable(paint)
rp6502_xram(xram.h "XRAM_.*")
rp6502_asset(paint help paint.hlp)
rp6502_asset(paint logo ${CMAKE_CURRENT_BINARY_DIR}/logo.bin)
rp6502_executable(paint DATA default RESET default)
target_sources(paint PRIVATE
    paint.c
)
