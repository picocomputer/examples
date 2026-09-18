add_executable(furelise)
rp6502_xram(xram.h "XRAM_.*")
rp6502_asset(furelise help furelise.hlp)
rp6502_executable(furelise DATA default RESET default)
target_sources(furelise PRIVATE
    furelise.c
    ezpsg.c
)
