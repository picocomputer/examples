cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(rp6502-sdk)

add_executable(rand)
target_sources(rand PRIVATE
    src/rand.c
)
target_link_libraries(rand PRIVATE
    rp6502
)

add_executable(brot16)
target_sources(brot16 PRIVATE
    src/brot16.c
)
target_link_libraries(brot16 PRIVATE
    rp6502
)

add_executable(brot32)
target_sources(brot32 PRIVATE
    src/brot32.c
)
target_link_libraries(brot32 PRIVATE
    rp6502
)

add_executable(vsync)
target_sources(vsync PRIVATE
    src/vsync.c
)
target_link_libraries(vsync PRIVATE
    rp6502
)

add_executable(palette)
target_sources(palette PRIVATE
    src/palette.c
)
target_link_libraries(palette PRIVATE
    rp6502
)
