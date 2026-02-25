# Add cmake commands: rp6502_executable() and rp6502_asset()
cmake_minimum_required(VERSION 3.17)

# Package the target as an RP6502 ROM.
#
# RP6502 Executable ROM
# ^^^^^^^^^^^^^^^^^^^^^
#
#  rp6502_executable(<name>
#                    [DATA <addr>]
#                    [NMI <addr>]
#                    [RESET <addr>]
#                    [IRQ <addr>]
#                    roms...)
#
# Packages the ``<name>`` into RP6502 ROM format merged with
# additional asset ``roms...`` created with rp6502_asset().
# The word `file` may be used for any <addr> indicating the
# address is to be read from the linker output in this order:
# ``DATA <addr>`` Starting memory address to load data.
# ``NMI <addr>`` Address for NMI to be stored at $FFFA-$FFFB.
# ``RESET <addr>`` Address for RESET to be stored at $FFFC-$FFFD.
# ``IRQ <addr>`` Address for IRQ to be stored at $FFFE-$FFFF.
#
function(rp6502_executable name)
    # Parse args
    set(data_addr "none")
    set(nmi_addr "none")
    set(reset_addr "none")
    set(irq_addr "none")
    set(extra_roms)
    foreach(X IN LISTS ARGN)
        if (NOT data_addr)
            set(data_addr ${X})
        elseif (NOT reset_addr)
            set(reset_addr ${X})
        elseif (NOT irq_addr)
            set(irq_addr ${X})
        elseif (NOT nmi_addr)
            set(nmi_addr ${X})
        elseif (X STREQUAL "DATA")
            set(data_addr FALSE)
        elseif (X STREQUAL "RESET")
            set(reset_addr FALSE)
        elseif (X STREQUAL "IRQ")
            set(irq_addr FALSE)
        elseif (X STREQUAL "NMI")
            set(nmi_addr FALSE)
        else ()
            list(APPEND extra_roms ${X})
        endif ()
    endforeach()
    # Resolve relative extra_roms against current source dir
    set(all_extra_roms)
    foreach(rom IN LISTS extra_roms)
        if (IS_ABSOLUTE "${rom}")
            list(APPEND all_extra_roms "${rom}")
        else()
            list(APPEND all_extra_roms "${CMAKE_CURRENT_SOURCE_DIR}/${rom}")
        endif()
    endforeach()
    # Collect asset ROMs registered by rp6502_asset()
    get_target_property(asset_roms ${name} RP6502_ASSET_ROMS)
    if (asset_roms)
        list(APPEND all_extra_roms ${asset_roms})
    endif()
    # Remove old ROM
    add_custom_command(TARGET ${name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -f
        "${CMAKE_CURRENT_BINARY_DIR}/${name}.rp6502"
    )
    # Create new ROM
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(tool_command "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/rp6502.py"
    )
    if (NOT data_addr STREQUAL "none")
        list(APPEND tool_command
            -a "${data_addr}"
        )
    else ()
        message (FATAL_ERROR "rp6502_executable DATA address missing")
    endif ()
    if (NOT nmi_addr STREQUAL "none")
        list(APPEND tool_command
            -n "${nmi_addr}"
        )
    endif ()
    if (NOT reset_addr STREQUAL "none")
        list(APPEND tool_command
            -r "${reset_addr}"
        )
    else ()
        message (FATAL_ERROR "rp6502_executable RESET address missing")
    endif ()
    if (NOT irq_addr STREQUAL "none")
        list(APPEND tool_command
            -i "${irq_addr}"
        )
    endif ()
    list(APPEND tool_command
        -o "${CMAKE_CURRENT_BINARY_DIR}/${name}.rp6502"
        create "${CMAKE_CURRENT_BINARY_DIR}/${name}"
        -- ${all_extra_roms}
    )
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${tool_command}
    )
    set_property(TARGET ${name} APPEND PROPERTY
        LINK_DEPENDS ${all_extra_roms}
    )
endfunction()

# Package anything as an RP6502 ROM.
#
# RP6502 Asset ROM
# ^^^^^^^^^^^^^^^^
#
#  rp6502_asset(<name> addr in_file)
#
# Packages the ``in_file`` into RP6502 ROM format.
# Output (.rp6502) lands in CMakeFiles/<name>.dir/ alongside object files.
# Use `none` for addr to produce a raw ROM with no load address.
#
function(rp6502_asset name addr in_file)
    get_filename_component(out_name "${in_file}" NAME)
    set(out_file "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}.rp6502/${out_name}")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(asset_command
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/rp6502.py"
    )
    if (NOT addr STREQUAL "none")
        list(APPEND asset_command -a "${addr}")
    endif()
    list(APPEND asset_command
        -o "${out_file}"
        create "${CMAKE_CURRENT_SOURCE_DIR}/${in_file}"
    )
    add_custom_command(
        OUTPUT "${out_file}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${in_file}"
        COMMAND ${asset_command}
    )
    target_sources(${name} PRIVATE "${out_file}")
    set_property(TARGET ${name} APPEND PROPERTY
        RP6502_ASSET_ROMS "${out_file}"
    )
endfunction()
