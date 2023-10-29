/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "rp6502.h"

// Obligatory Mandelbrot Example
// https://en.wikipedia.org/wiki/Mandelbrot_set

// This version optimized for fixed point math on 8-bit processors
typedef int32_t fint32_t;
#define FRAC_BITS 12
#define FINT32(whole, frac) (((fint32_t)whole << FRAC_BITS) | (frac >> (16 - FRAC_BITS)))

#define WIDTH 320
#define HEIGHT 240 // 180 or 240

static void erase()
{
    unsigned i;
    // Erase console
    printf("\f");
    // Erase graphics
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i = 0x1300; --i;)
    {
        // Partially unrolled loop is FAST
        RIA.rw0 = 255;
        RIA.rw0 = 255;
        RIA.rw0 = 255;
        RIA.rw0 = 255;
        RIA.rw0 = 255;
        RIA.rw0 = 255;
        RIA.rw0 = 255;
        RIA.rw0 = 255;
    }
    RIA.addr0 = 0;
    for (i = 0x1300; --i;)
    {
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = 0;
    }
}

void mandelbrot()
{
    int8_t vbyte;
    int16_t px, py;
    RIA.addr0 = 0;
    for (py = 0; py < HEIGHT; ++py)
    {
        for (px = 0; px < WIDTH; ++px)
        {
#if (HEIGHT == 180)
            fint32_t x0 = px * FINT32(3, 32768u) / WIDTH - FINT32(2, 32768u); // -2.5-1
#else
            fint32_t x0 = px * FINT32(3, 0u) / WIDTH - FINT32(2, 16384u);
#endif
            fint32_t y0 = py * FINT32(2, 15728u) / HEIGHT - FINT32(1, 7864u); // +-1.12
            fint32_t x = 0;
            fint32_t y = 0;
            int8_t iteration = 0;
            for (iteration = 0; iteration < 16; ++iteration)
            {
                fint32_t xtemp;
                fint32_t xx = x * x >> FRAC_BITS;
                fint32_t yy = y * y >> FRAC_BITS;
                if (xx + yy > FINT32(4, 0))
                    break;
                xtemp = xx - yy + x0;
                y = (x * y >> (FRAC_BITS - 1)) + y0;
                x = xtemp;
            }
            iteration = iteration - 1;
            if (px & 1)
                RIA.rw0 = vbyte | (iteration << 4);
            else
                vbyte = iteration;
        }
    }
}

void main()
{
    erase();

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, true);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, true);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 320);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 240);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_canvas(1);
    xreg_vga_mode(3, 10, 0xFF00);
    xreg_vga_mode(0, 1); // console

    printf("Mandelbrot Set");
    mandelbrot();

    printf("\nPress any key to exit");
    xreg_ria_keyboard(0xFF10);
    RIA.addr0 = 0xFF10;
    RIA.step0 = 0;
    while (RIA.rw0 & 1)
        ;
    printf("\n");
}
