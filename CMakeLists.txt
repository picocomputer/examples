cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(rp6502-sdk)

add_executable(mandelbrot)
rp6502_executable(mandelbrot)
target_sources(mandelbrot PRIVATE
    src/mandelbrot.c
)
target_link_libraries(mandelbrot PRIVATE
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

add_executable(vsync)
rp6502_executable(vsync)
target_sources(vsync PRIVATE
    src/vsync.c
)
target_link_libraries(vsync PRIVATE
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

add_executable(clock)
rp6502_executable(clock)
target_sources(clock PRIVATE
    src/clock.c
)
target_link_libraries(clock PRIVATE
    rp6502
)

add_executable(set-time)
rp6502_executable(set-time)
target_sources(set-time PRIVATE
    src/set-time.c
)
target_link_libraries(set-time PRIVATE
    rp6502
)

add_executable(print-time)
rp6502_executable(print-time)
target_sources(print-time PRIVATE
    src/print-time.c
)
target_link_libraries(print-time PRIVATE
    rp6502
)
