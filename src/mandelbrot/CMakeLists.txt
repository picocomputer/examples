add_executable(mandelbrot)
rp6502_align(mandelbrot "XRAM_.*")
rp6502_asset(mandelbrot help mandelbrot.hlp)
rp6502_executable(mandelbrot DATA default RESET default)
rp6502_web(mandelbrot CONFIG [[ title: 'Mandelbrot', overlay: '' ]])
target_sources(mandelbrot PRIVATE
    mandelbrot.c
)
