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
#define SPRITE_LENGTH 10

void main()
{
    unsigned u;
    uint8_t v;
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
    struct
    {
        int v;
        int d;
    } scale[SPRITE_LENGTH];

    // Copy sprite data
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (u = 0; u < sizeof(raspberry_128x128); u++)
        RIA.rw0 = raspberry_128x128[u];

    // Initial sprite positions
    for (u = 0; u < SPRITE_LENGTH; u++)
    {

        unsigned ptr = SPRITE_CONFIG + u * sizeof(vga_mode4_asprite_t);
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
        scale[u].v = (((uint32_t)rand() * 512) >> 15) + 256;
        if (((uint32_t)rand() * 2) >> 15)
            scale[u].d = 3;
        else
            scale[u].d = -3;
        xram0_struct_set(ptr, vga_mode4_asprite_t, transform[0], scale[u].v);
        xram0_struct_set(ptr, vga_mode4_asprite_t, transform[1], 0);
        xram0_struct_set(ptr, vga_mode4_asprite_t, transform[2], 0);
        xram0_struct_set(ptr, vga_mode4_asprite_t, transform[3], 0);
        xram0_struct_set(ptr, vga_mode4_asprite_t, transform[4], scale[u].v);
        xram0_struct_set(ptr, vga_mode4_asprite_t, transform[5], 0);
        xram0_struct_set(ptr, vga_mode4_asprite_t, x_pos_px, x);
        xram0_struct_set(ptr, vga_mode4_asprite_t, y_pos_px, y);
        xram0_struct_set(ptr, vga_mode4_asprite_t, xram_sprite_ptr, 0x0000);
        xram0_struct_set(ptr, vga_mode4_asprite_t, log_size, 7);
        xram0_struct_set(ptr, vga_mode4_asprite_t, has_opacity_metadata, true);
    }

    // Program VGA
    xreg_vga_canvas(1);
    xreg_vga_mode(0);
    xreg_vga_mode(4, 1, SPRITE_CONFIG, SPRITE_LENGTH);

    // Vsync loop
    v = RIA.vsync;
    while (1)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;

        // Copy positions during vblank
        RIA.addr0 = SPRITE_CONFIG + 12;
        RIA.step0 = sizeof(vga_mode4_asprite_t);
        RIA.addr1 = SPRITE_CONFIG + 13;
        RIA.step1 = sizeof(vga_mode4_asprite_t);
        for (u = 0; u < SPRITE_LENGTH; u++)
        {
            int val = sprites[u].x;
            RIA.rw0 = val & 0xff;
            RIA.rw1 = (val >> 8) & 0xff;
        }
        RIA.addr0 = SPRITE_CONFIG + 14;
        RIA.addr1 = SPRITE_CONFIG + 15;
        for (u = 0; u < SPRITE_LENGTH; u++)
        {
            int val = sprites[u].y;
            RIA.rw0 = val & 0xff;
            RIA.rw1 = (val >> 8) & 0xff;
        }

        // explode
        for (u = 0; u < SPRITE_LENGTH; u++)
        {
            unsigned ptr = SPRITE_CONFIG + u * sizeof(vga_mode4_asprite_t);
            scale[u].v += scale[u].d;
            if (scale[u].v < 256 || scale[u].v > 768)
                scale[u].d *= -1;
            xram0_struct_set(ptr, vga_mode4_asprite_t, transform[0], scale[u].v);
            xram0_struct_set(ptr, vga_mode4_asprite_t, transform[4], scale[u].v);
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
