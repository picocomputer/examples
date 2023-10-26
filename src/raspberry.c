/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
        int xv;
        int yv;
    } sprites[SPRITE_LENGTH];

    xreg_vga_canvas(1);

    for (u = 0; u < SPRITE_LENGTH; u++)
    {
        unsigned ptr = SPRITE_CONFIG + u * sizeof(vga_mode4_sprite_t);
        unsigned x = ((uint32_t)rand() * (320 - 128)) >> 15;
        unsigned y = ((uint32_t)rand() * (240 - 128)) >> 15;
        sprites[u].x = x;
        sprites[u].y = y;
        if (((uint32_t)rand() * 2) >> 15)
            sprites[u].xv = 1;
        else
            sprites[u].xv = -1;
        if (((uint32_t)rand() * 2) >> 15)
            sprites[u].yv = 1;
        else
            sprites[u].yv = -1;
        xram0_struct_set(ptr, vga_mode4_sprite_t, x_pos_px, x);
        xram0_struct_set(ptr, vga_mode4_sprite_t, y_pos_px, y);
        xram0_struct_set(ptr, vga_mode4_sprite_t, xram_sprite_ptr, 0x0000);
        xram0_struct_set(ptr, vga_mode4_sprite_t, log_size, 7);
        xram0_struct_set(ptr, vga_mode4_sprite_t, has_opacity_metadata, false);
    }

    xreg_vga_mode(0);
    xreg_vga_mode(4, 0, SPRITE_CONFIG, SPRITE_LENGTH);

    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (u = 0; u < sizeof(raspberry_128x128); u++)
        RIA.rw0 = raspberry_128x128[u];

    v = RIA.vsync;
    while (1)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;

        for (u = 0; u < SPRITE_LENGTH; u++)
        {
            unsigned ptr = SPRITE_CONFIG + u * sizeof(vga_mode4_sprite_t);
            xram0_struct_set(ptr, vga_mode4_sprite_t, x_pos_px, sprites[u].x);
            xram0_struct_set(ptr, vga_mode4_sprite_t, y_pos_px, sprites[u].y);
        }

        for (u = 0; u < SPRITE_LENGTH; u++)
        {
            int x = sprites[u].x = sprites[u].x + sprites[u].xv;
            int y = sprites[u].y = sprites[u].y + sprites[u].yv;
            if (x < 0 || x > 320 - 128)
                sprites[u].xv *= -1;
            if (y < 0 || y > 240 - 128)
                sprites[u].yv *= -1;
        }
    }
}
