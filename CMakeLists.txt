cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(rp6502-sdk)

add_executable(mandelbrot)
target_sources(mandelbrot PRIVATE
    src/mandelbrot.c
)
target_link_libraries(mandelbrot PRIVATE
    rp6502
)

add_executable(palette)
target_sources(palette PRIVATE
    src/palette.c
)
target_link_libraries(palette PRIVATE
    rp6502
)

add_executable(vsync)
target_sources(vsync PRIVATE
    src/vsync.c
)
target_link_libraries(vsync PRIVATE
    rp6502
)

add_executable(vgaprog)
target_sources(vgaprog PRIVATE
    src/vgaprog.c
)
target_link_libraries(vgaprog PRIVATE
    rp6502
)
