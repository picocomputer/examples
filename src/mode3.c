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
#include <fcntl.h>

// This is not good code to learn from,
// it's a functional test for developing mode3.

void clear()
{
    unsigned i;
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i = 0; i < 61440u / 8; i++)
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

void box(unsigned qty, uint8_t bpp)
{
    srand(6);
    while (qty--)
    {
        int color, x1, y1, x2, y2, x, y;
        color = rand();
        x1 = ((uint32_t)rand() * 240) >> 15;
        y1 = ((uint32_t)rand() * 128) >> 15;
        x2 = ((uint32_t)rand() * 240) >> 15;
        y2 = ((uint32_t)rand() * 128) >> 15;
        if (x1 > x2)
        {
            x = x1;
            x1 = x2;
            x2 = x;
        }
        if (y1 > y2)
        {
            y = y1;
            y1 = y2;
            y2 = y;
        }

        switch (bpp)
        {
        case 1:
            RIA.step0 = 1;
            for (y = y1; y < y2; y++)
            {
                RIA.addr0 = 240 / 8 * y + x1 / 8;
                for (x = x1 / 8; x < x2 / 8; x++)
                    RIA.rw0 = 0xFF;
            }
            break;
        case 2:
            RIA.step0 = 0;
            for (y = y1; y < y2; y++)
            {
                for (x = x1; x < x2; x++)
                {
                    uint8_t shift = 2 * (3 - (x & 3));
                    RIA.addr0 = 240 / 4 * y + x / 4;
                    RIA.rw0 = (RIA.rw0 & ~(3 << shift)) | ((color & 3) << shift);
                }
            }
            break;
        case 4:
            RIA.step0 = 0;
            for (y = y1; y < y2; y++)
            {
                for (x = x1; x < x2; x++)
                {
                    uint8_t shift = 4 * (1 - (x & 1));
                    RIA.addr0 = 240 / 2 * y + x / 2;
                    RIA.rw0 = (RIA.rw0 & ~(7 << shift)) | ((color & 7) << shift);
                }
            }
            break;
        case 8:
            RIA.step0 = 1;
            for (y = y1; y < y2; y++)
            {
                RIA.addr0 = 240 * y + x1;
                for (x = x1; x < x2; x++)
                    RIA.rw0 = color;
            }
            break;
        case 16:
            RIA.step0 = 1;
            for (y = y1; y < y2; y++)
            {
                RIA.addr0 = 480 * y + x1 * 2;
                for (x = x1; x < x2; x++)
                {
                    RIA.rw0 = color;
                    RIA.rw0 = color >> 8;
                }
            }
            break;
        }
    }
}

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
            xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, x);
            if (++x >= 240)
                x = 0;
        }
        if (y_scroll)
        {
            xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, y);
            if (++y >= 128)
                y = 0;
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

void main()
{
    xreg_vga_canvas(2);
    clear();

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, true);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, true);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 240);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 128);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(0, 1, 172, 180);
    xreg_ria_keyboard(0xFF10);

    printf("\n1bpp (press any key)");
    xreg_vga_mode(3, 0, 0xFF00, 0, 0, 172);
    clear();
    box(5, 1);
    scroll(true, true);

    printf("\n2bpp (press any key)");
    xreg_vga_mode(3, 1, 0xFF00, 0, 0, 172);
    clear();
    box(10, 2);
    scroll(true, true);

    printf("\n4bpp (press any key)");
    xreg_vga_mode(3, 2, 0xFF00, 0, 0, 172);
    clear();
    box(10, 4);
    scroll(true, true);

    printf("\n8bpp (press any key)");
    xreg_vga_mode(3, 3, 0xFF00, 0, 0, 172);
    clear();
    box(10, 8);
    scroll(true, true);

    printf("\n16bpp (press any key)");
    xreg_vga_mode(3, 4, 0xFF00, 0, 0, 172);
    clear();
    box(10, 16);
    scroll(true, true);

    printf("\n");
}
