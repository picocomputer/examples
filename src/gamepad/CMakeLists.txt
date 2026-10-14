add_executable(gamepad)
rp6502_align(gamepad "XRAM_.*")
rp6502_asset(gamepad help gamepad.hlp)
rp6502_executable(gamepad DATA default RESET default)
rp6502_web(gamepad
    CONFIG [[ title: 'Gamepads', border: '1em', overlay: '' ]]
)
target_sources(gamepad PRIVATE
    gamepad.c
)
