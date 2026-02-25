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

// Example where assets are files in the ROM:
//   rp6502_asset(altair altair.pal src/altair.pal.bin)
//   rp6502_asset(altair altair.dat src/altair.dat.bin)
// These can be loaded from the "ROM:" drive.

static void load_asset(const char *path, unsigned xram_addr)
{
    int fd;
    int bytes_read;
    printf("Loading %s at $%04X\n", path, xram_addr);
    fd = open(path, O_RDONLY);
    while ((bytes_read = read_xram(xram_addr, 0x7FFF, fd)) > 0)
        xram_addr += bytes_read;
    close(fd);
}

void main()
{
    load_asset("ROM:altair.pal", 0x0000);
    load_asset("ROM:altair.dat", 0x0200);

    // Mode 3 config
    printf("Configuring VGA\n");
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 320);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 180);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0200);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0x0000);

    // Program the VGA
    printf("Programming VGA\n");
    xreg_vga_canvas(2);          // Canvas
    xreg_vga_mode(3, 3, 0xFF00); // Mode 3
    xreg_vga_mode(0, 1);         // Mode 0

    // Wait forever, or we'll drop back to console on exit.
    printf("\fPress CTRL-ALT-DEL when done viewing");
    while (1)
        ;
}
