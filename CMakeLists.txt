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

add_executable(vgaprog)
rp6502_executable(vgaprog)
target_sources(vgaprog PRIVATE
    src/vgaprog.c
)
target_link_libraries(vgaprog PRIVATE
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
