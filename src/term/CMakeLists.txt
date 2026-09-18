add_executable(term)
rp6502_asset(term help term.hlp)
rp6502_executable(term DATA default RESET default)
target_sources(term PRIVATE
    term.c
)
