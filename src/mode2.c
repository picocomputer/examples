/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// This is not good code to learn from,
// it's a functional test for developing mode2.

void scroll(bool x_scroll, bool y_scroll)
{
    int x = 0, y = 0;
    uint8_t v = RIA.vsync;
    while (1)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;
        if (x_scroll)
        {
            xram0_struct_set(0xFF00, vga_mode2_config_t, x_pos_px, x);
            if (++x >= 640)
                x = -640;
        }
        if (y_scroll)
        {
            xram0_struct_set(0xFF00, vga_mode2_config_t, y_pos_px, y);
            if (++y >= 480)
                y = -480;
        }

        RIA.addr0 = 0xFF10;
        RIA.step0 = 0;
        if (!(RIA.rw0 & 1))
        {
            while (!(RIA.rw0 & 1))
                ;
            return;
        }
    }
}

void option0()
{
    int i;

    xreg_vga_mode(2, 0, 0xFF00);
    RIA.addr0 = 0x1000;
    RIA.step0 = 1;
    // 0
    RIA.rw0 = 1;
    RIA.rw0 = 2;
    RIA.rw0 = 4;
    RIA.rw0 = 8;
    RIA.rw0 = 16;
    RIA.rw0 = 32;
    RIA.rw0 = 64;
    RIA.rw0 = 128;
    // 1
    RIA.rw0 = 128;
    RIA.rw0 = 64;
    RIA.rw0 = 32;
    RIA.rw0 = 16;
    RIA.rw0 = 8;
    RIA.rw0 = 4;
    RIA.rw0 = 2;
    RIA.rw0 = 1;

    _randomize();
    RIA.addr0 = 0;
    for (i = 0; i < 40 * 30; i++)
    {
        if (rand() < 16384)
            RIA.rw0 = 0;
        else
            RIA.rw0 = 1;
    }
}

void option8()
{
    int i;

    xreg_vga_mode(2, 8, 0xFF00);

    RIA.addr0 = 0x1000;
    RIA.step0 = 1;

    // 0 "X"
    RIA.rw0 = 1;
    RIA.rw0 = 128;

    RIA.rw0 = 2;
    RIA.rw0 = 64;

    RIA.rw0 = 4;
    RIA.rw0 = 32;

    RIA.rw0 = 8;
    RIA.rw0 = 16;

    RIA.rw0 = 16;
    RIA.rw0 = 8;

    RIA.rw0 = 32;
    RIA.rw0 = 4;

    RIA.rw0 = 64;
    RIA.rw0 = 2;

    RIA.rw0 = 128;
    RIA.rw0 = 1;

    //
    RIA.rw0 = 128;
    RIA.rw0 = 1;

    RIA.rw0 = 64;
    RIA.rw0 = 2;

    RIA.rw0 = 32;
    RIA.rw0 = 4;

    RIA.rw0 = 16;
    RIA.rw0 = 8;

    RIA.rw0 = 8;
    RIA.rw0 = 16;

    RIA.rw0 = 4;
    RIA.rw0 = 32;

    RIA.rw0 = 2;
    RIA.rw0 = 64;

    RIA.rw0 = 1;
    RIA.rw0 = 128;

    // 1 "diamond"

    RIA.rw0 = 128;
    RIA.rw0 = 1;

    RIA.rw0 = 64;
    RIA.rw0 = 2;

    RIA.rw0 = 32;
    RIA.rw0 = 4;

    RIA.rw0 = 16;
    RIA.rw0 = 8;

    RIA.rw0 = 8;
    RIA.rw0 = 16;

    RIA.rw0 = 4;
    RIA.rw0 = 32;

    RIA.rw0 = 2;
    RIA.rw0 = 64;

    RIA.rw0 = 1;
    RIA.rw0 = 128;

    //
    RIA.rw0 = 1;
    RIA.rw0 = 128;

    RIA.rw0 = 2;
    RIA.rw0 = 64;

    RIA.rw0 = 4;
    RIA.rw0 = 32;

    RIA.rw0 = 8;
    RIA.rw0 = 16;

    RIA.rw0 = 16;
    RIA.rw0 = 8;

    RIA.rw0 = 32;
    RIA.rw0 = 4;

    RIA.rw0 = 64;
    RIA.rw0 = 2;

    RIA.rw0 = 128;
    RIA.rw0 = 1;

    _randomize();
    RIA.addr0 = 0;
    for (i = 0; i < 40 * 30; i++)
    {
        if (rand() < 16384)
            RIA.rw0 = 0;
        else
            RIA.rw0 = 1;
    }
}

void main()
{
    xreg_vga_canvas(1);

    xram0_struct_set(0xFF00, vga_mode2_config_t, x_wrap, true);
    xram0_struct_set(0xFF00, vga_mode2_config_t, y_wrap, true);
    xram0_struct_set(0xFF00, vga_mode2_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode2_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode2_config_t, width_tiles, 40);
    xram0_struct_set(0xFF00, vga_mode2_config_t, height_tiles, 30);
    xram0_struct_set(0xFF00, vga_mode2_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode2_config_t, xram_palette_ptr, 0xFFFF);
    xram0_struct_set(0xFF00, vga_mode2_config_t, xram_tile_ptr, 0x1000);

    xreg_ria_keyboard(0xFF10);

    option0();
    scroll(true, true);
    option8();
    scroll(true, true);

    printf("\n");
}
