add_executable(furelise)
rp6502_map(furelise xram.h "XRAM_.*")
rp6502_asset(furelise help furelise.hlp)
rp6502_executable(furelise DATA default RESET default)
rp6502_web(furelise CONFIG [[ title: 'Fur Elise', border: '1em', autoplay: 'auto' ]])
target_sources(furelise PRIVATE
    furelise.c
    ezpsg.c
)
