/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void main()
{
    int fd;
    unsigned xram_addr;
    int bytes_read;

    // Load palette and pixel data from raw ROM data into xram.
    fd = open("ROM:", O_RDONLY);
    xram_addr = 0;
    while ((bytes_read = read_xram(xram_addr, 0x7FFF, fd)) > 0)
        xram_addr += bytes_read;
    close(fd);

    // Mode 3 config
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 320);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 180);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0200);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0x0000);

    // Program the VGA
    xreg_vga_canvas(2);          // Canvas
    xreg_vga_mode(3, 3, 0xFF00); // Mode 3
    xreg_vga_mode(0, 1);         // Mode 0

    // Wait forever, or we'll drop back to console on exit.
    printf("\fPress CTRL-ALT-DEL when done viewing");
    while (1)
        ;
}
