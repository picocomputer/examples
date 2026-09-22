add_executable(mandelbrot)
rp6502_xram(mandelbrot xram.h "XRAM_.*")
rp6502_asset(mandelbrot help mandelbrot.hlp)
rp6502_executable(mandelbrot DATA default RESET default)
target_sources(mandelbrot PRIVATE
    mandelbrot.c
)
