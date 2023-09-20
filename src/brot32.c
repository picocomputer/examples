/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
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
    // Partially unrolled loop is FAST
    unsigned i = 0;
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i = 0x1300; --i;)
    {
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
    RIA.addr0 = 0;
}

static void wait()
{
    uint8_t discard;
    discard = RIA.rx;
    while (RIA.ready & RIA_READY_RX_BIT)
        discard = RIA.rx;
    while (!(RIA.ready & RIA_READY_RX_BIT))
        ;
    discard = RIA.rx;
}

void mandelbrot()
{
    int8_t vbyte;
    int16_t px, py;
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
    int i;
    typedef struct
    {
        int16_t xpos_px;
        int16_t ypos_px;
        int16_t width_px;
        int16_t height_px;
        uint16_t xram_data_ptr;
        uint16_t xram_palette_ptr;
    } mode3_config_t;
    mode3_config_t config = {};
    config.xpos_px = 0;
    config.ypos_px = 0;
    config.width_px = 320;
    config.height_px = 240;
    config.xram_data_ptr = 0;
    config.xram_palette_ptr = 0xFFFF;
    RIA.addr0 = 0xFF00;
    RIA.step0 = 1;
    for (i = 0; i < sizeof(mode3_config_t); i++)
        RIA.rw0 = ((uint8_t *)&config)[i];

    i = ria_xreg(1, 0, 0, 1);
    printf("canvas=%d\n", i);
    i = ria_xreg(1, 0, 1, 3, 0xFF00, 17);
    printf("mode_3=%d\n", i);
    i = ria_xreg(1, 0, 1, 0, 1);
    printf("mode_0=%d\n", i);

    // wait();
    while (1) // 0 run once, 1 loop forever
    {
        erase();
        mandelbrot();
        wait();
    }
}
