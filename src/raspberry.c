/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "raspberry_128x128_bgar5515.h"

#define SPRITE_CONFIG 0xF000
#define SPRITE_LENGTH 24

struct
{
    int x;
    int y;
} sprites[SPRITE_LENGTH];
struct
{
    int xv;
    int yv;
} vectors[SPRITE_LENGTH];

void main()
{
    unsigned u;
    unsigned char c;
    uint8_t v;

    // Copy sprite data
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (u = 0; u < sizeof(raspberry_128x128); u++)
        RIA.rw0 = raspberry_128x128[u];

    // Initial sprite positions
    for (u = 0; u < SPRITE_LENGTH; u++)
    {
        unsigned ptr = SPRITE_CONFIG + u * sizeof(vga_mode4_sprite_t);
        unsigned x = ((uint32_t)rand() * (320 - 128)) >> 15;
        unsigned y = ((uint32_t)rand() * (240 - 128)) >> 15;
        sprites[u].x = x;
        sprites[u].y = y;
        if (((uint32_t)rand() * 2) >> 15)
            vectors[u].xv = 1;
        else
            vectors[u].xv = -1;
        if (((uint32_t)rand() * 2) >> 15)
            vectors[u].yv = 1;
        else
            vectors[u].yv = -1;
        xram0_struct_set(ptr, vga_mode4_sprite_t, x_pos_px, x);
        xram0_struct_set(ptr, vga_mode4_sprite_t, y_pos_px, y);
        xram0_struct_set(ptr, vga_mode4_sprite_t, xram_sprite_ptr, 0x0000);
        xram0_struct_set(ptr, vga_mode4_sprite_t, log_size, 7);
        xram0_struct_set(ptr, vga_mode4_sprite_t, has_opacity_metadata, true);
    }

    // Program VGA
    xreg_vga_canvas(1);
    xreg_vga_mode(0);
    xreg_vga_mode(4, 0, SPRITE_CONFIG, SPRITE_LENGTH);

    // Vsync loop
    v = RIA.vsync;
    while (1)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;

        // Copy positions during vblank
        RIA.step0 = sizeof(vga_mode4_sprite_t);
        RIA.step1 = sizeof(vga_mode4_sprite_t);
        RIA.addr0 = SPRITE_CONFIG;
        RIA.addr1 = SPRITE_CONFIG + 1;
        for (c = 0; c < SPRITE_LENGTH; c++)
        {
            int val = sprites[c].x;
            RIA.rw0 = val & 0xff;
            RIA.rw1 = val >> 8;
        }
        RIA.addr0 = SPRITE_CONFIG + 2;
        RIA.addr1 = SPRITE_CONFIG + 3;
        for (c = 0; c < SPRITE_LENGTH; c++)
        {
            int val = sprites[c].y;
            RIA.rw0 = val & 0xff;
            RIA.rw1 = (val >> 8) & 0xff;
        }

        // Update positions
        for (u = 0; u < SPRITE_LENGTH; u++)
        {
            int x = sprites[u].x = sprites[u].x + vectors[u].xv;
            int y = sprites[u].y = sprites[u].y + vectors[u].yv;
            if (x < -12 || x > 320 - 118)
                vectors[u].xv *= -1;
            if (y < 0 || y > 240 - 128)
                vectors[u].yv *= -1;
        }
    }
}
