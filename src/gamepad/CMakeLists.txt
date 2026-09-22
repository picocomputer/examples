add_executable(gamepad)
rp6502_xram(gamepad xram.h "XRAM_.*")
rp6502_asset(gamepad help gamepad.hlp)
rp6502_executable(gamepad DATA default RESET default)
target_sources(gamepad PRIVATE
    gamepad.c
)
