# Wrapper script to filter out arguments that cc65 doesn't support.
# This enables passing extra arguments to IntelliSense.

# Args 0-3 are the cmake call to this script
if(NOT CMAKE_ARGV3 STREQUAL "--")
    message(FATAL_ERROR "No -- separator found in arguments")
endif()

# First argument after -- is CC65_COMPILER
set(CC65_COMPILER "${CMAKE_ARGV4}")
if(NOT EXISTS "${CC65_COMPILER}")
    message(FATAL_ERROR "Could not find executable: ${CC65_COMPILER}")
endif()

# Remove -include and its argument
set(FILTERED_ARGS "")
set(SKIP_NEXT FALSE)
foreach(INDEX RANGE 5 ${CMAKE_ARGC})
    if(DEFINED CMAKE_ARGV${INDEX})
        set(ARG "${CMAKE_ARGV${INDEX}}")
        if(SKIP_NEXT)
            set(SKIP_NEXT FALSE)
        elseif(ARG STREQUAL "-include")
            set(SKIP_NEXT TRUE)
        else()
            list(APPEND FILTERED_ARGS "${ARG}")
        endif()
    endif()
endforeach()

# Execute the real compiler with filtered arguments
execute_process(
    COMMAND ${CC65_COMPILER} ${FILTERED_ARGS}
    RESULT_VARIABLE RESULT
)

# Propagate the exit code without any CMake error message
if(NOT RESULT EQUAL 0)
    execute_process(COMMAND ${CMAKE_COMMAND} -E false)
endif()
