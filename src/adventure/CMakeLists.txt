if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/troglobit/src/advent.c)
    execute_process(
        COMMAND git submodule update --init troglobit
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/troglobit/src/advent.c)
    message(FATAL_ERROR "Adventure needs its submodule:\n"
        "  git submodule update --init src/adventure/troglobit")
endif()

add_executable(adventure)
rp6502_asset(adventure help adventure.hlp)
rp6502_asset(adventure /advent1.txt troglobit/src/advent1.txt)
rp6502_asset(adventure /advent2.txt troglobit/src/advent2.txt)
rp6502_asset(adventure /advent3.txt troglobit/src/advent3.txt)
rp6502_asset(adventure /advent4.txt troglobit/src/advent4.txt)
rp6502_executable(adventure DATA default RESET default)
target_include_directories(adventure PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_sources(adventure PRIVATE
    err.c
    troglobit/src/advent.c
    troglobit/src/database.c
    troglobit/src/english.c
    troglobit/src/itverb.c
    troglobit/src/turn.c
    troglobit/src/verb.c
)
