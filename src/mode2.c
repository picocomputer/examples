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
// it's a functional test for developing mode1.

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
            xram0_struct_set(0xFF00, vga_mode1_config_t, x_pos_px, x);
            if (++x >= 320)
                x = 0;
        }
        if (y_scroll)
        {
            xram0_struct_set(0xFF00, vga_mode1_config_t, y_pos_px, y);
            if (++y >= 480)
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
    int i;
    char message[] = "                                        "
                     "    **** COMMODORE 64 BASIC V2 ****     "
                     "                                        "
                     " 64K RAM SYSTEM  38911 BASIC BYTES FREE "
                     "                                        "
                     "READY                                  ";

    xreg_vga_canvas(3);

    xram0_struct_set(0xFF00, vga_mode1_config_t, x_wrap, true);
    xram0_struct_set(0xFF00, vga_mode1_config_t, y_wrap, true);
    xram0_struct_set(0xFF00, vga_mode1_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode1_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode1_config_t, width_chars, 40);
    xram0_struct_set(0xFF00, vga_mode1_config_t, height_chars, 30);
    xram0_struct_set(0xFF00, vga_mode1_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode1_config_t, xram_palette_ptr, 0xFFFF);
    xram0_struct_set(0xFF00, vga_mode1_config_t, xram_font_ptr, 0xFFFF);

    xreg_vga_mode(1, 3, 0xFF00);
    xreg_ria_keyboard(0xFF10);


    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i = 0; i < 40 * 30; i++)
    {
        RIA.rw0 = ' ';
        RIA.rw0 = 14;
        RIA.rw0 = 12;
    }
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i = 0; i < sizeof(message); i++)
    {
        RIA.rw0 = message[i];
        RIA.rw0 = 14;
        RIA.rw0 = 12;
    }
    RIA.rw0 = ' ';
    RIA.rw0 = 12;
    RIA.rw0 = 14;

    // while (1)
    //     ;
    scroll(true, true);

    printf("\n");
}
