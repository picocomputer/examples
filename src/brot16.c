/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "rp6502.h"

// Obligatory Mandelbrot Example
// https://en.wikipedia.org/wiki/Mandelbrot_set

// This version optimized for fixed point math on 8-bit ptocessors
typedef int16_t fint16_t;
#define FRAC_BITS 6 // >6 not enough range, <6 is lost precision
#define FINT16(whole, frac) (((fint16_t)whole << FRAC_BITS) | (frac >> (8 - FRAC_BITS)))

#define WIDTH 64
#define HEIGHT 16

void main()
{
    static const char *const chr = " .:-~=+<!*O#@ ";
    int8_t max_iteration = strlen(chr);
    int16_t px, py;
    assert(max_iteration == 14);
    for (py = 0; py < HEIGHT; ++py)
    {
        for (px = 0; px < WIDTH; ++px)
        {
            fint16_t x0 = px * FINT16(3, 128) / WIDTH - FINT16(2, 128);
            fint16_t y0 = py * FINT16(2, 60) / HEIGHT - FINT16(1, 30);
            fint16_t x = 0;
            fint16_t y = 0;
            int8_t iteration = 0;
            for (iteration = 0; iteration < max_iteration; ++iteration)
            {
                fint16_t xtemp;
                fint16_t xx = x * x >> FRAC_BITS;
                fint16_t yy = y * y >> FRAC_BITS;
                fint16_t sum = xx + yy;
                if (sum > FINT16(4, 0))
                    break;
                xtemp = xx - yy + x0;
                y = (x * y >> (FRAC_BITS - 1)) + y0;
                x = xtemp;
            }
            printf("\33[4%cm%c\33[0m", '0' + iteration / 2, chr[iteration - 1]);
        }
        putchar(10);
    }
}
