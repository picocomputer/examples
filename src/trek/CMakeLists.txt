add_executable(trek)
rp6502_asset(trek help trek.hlp)
rp6502_asset(trek instructions.bas instructions.bas)
rp6502_asset(trek game.bas game.bas)
rp6502_basic(trek instructions.bas)
rp6502_web(trek CONFIG [[
    title: 'Super Star Trek',
    border: '1em',
    footer: 'Type a command, then press Enter. XXX resigns.',
]])
