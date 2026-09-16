find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/logo.bin
    COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/png2canvas.py
        ${CMAKE_CURRENT_SOURCE_DIR}/logo.png
        ${CMAKE_CURRENT_BINARY_DIR}/logo.bin
    DEPENDS png2canvas.py logo.png
    VERBATIM
)

rp6502_xram(xram.h "XRAM_.*" "XRAM_.*_DATA")

# The mouse is read from a VIA timer interrupt, which is cc65's set_irq().
if(NOT CMAKE_C_COMPILER_ID STREQUAL "cc65")
    message(WARNING "paint: skipped, llvm-mos has no set_irq()")
    return()
endif()

add_executable(paint)
rp6502_asset(paint help paint.hlp)
rp6502_asset(paint XRAM_CANVAS_DATA ${CMAKE_CURRENT_BINARY_DIR}/logo.bin)
rp6502_asset(paint logo ${CMAKE_CURRENT_BINARY_DIR}/logo.bin)
rp6502_executable(paint DATA default RESET default)
target_sources(paint PRIVATE
    paint.c
)
