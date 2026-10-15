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
rp6502_test(NAME connect COMMAND tests/connect.txt
    $<TARGET_PROPERTY:gamepad,RP6502_ROM>)
rp6502_test(NAME buttons COMMAND tests/buttons.txt
    $<TARGET_PROPERTY:gamepad,RP6502_ROM>)
