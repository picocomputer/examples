add_executable(nfc)
rp6502_asset(nfc help nfc.hlp)
rp6502_executable(nfc DATA default RESET default)
target_sources(nfc PRIVATE
    nfc.c
)
