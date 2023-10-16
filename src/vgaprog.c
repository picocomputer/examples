#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>

int load(const char *name)
{
    int fd, result;
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, -1000);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0200);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0x0000);
    fd = open(name, O_RDONLY);
    if (fd < 0)
        return fd;
    result = read_xram(0x0000, 0x6000, fd);
    if (result < 0)
        return result;
    result = read_xram(0x6000, 0x6000, fd);
    if (result < 0)
        return result;
    result = read_xram(0xC000, 0x2300, fd);
    if (result < 0)
        return result;
    result = close(fd);
    if (result < 0)
        return result;
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
}

void clear()
{
    unsigned i;
    RIA.addr0 = 0;
    RIA.step0 = 1;
    for (i = 0; i < 57600u; i++)
        RIA.rw0 = 0;
}

void box(unsigned qty)
{
    srand(6);
    while (qty--)
    {
        int color, x1, y1, x2, y2, x, y;
        color = rand();
        x1 = ((uint32_t)rand() * 320) >> 15;
        y1 = ((uint32_t)rand() * 180) >> 15;
        x2 = ((uint32_t)rand() * 320) >> 15;
        y2 = ((uint32_t)rand() * 180) >> 15;
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
        RIA.step1 = 0;
        for (y = y1; y < y2; y++)
        {
            RIA.addr0 = 320 * y + x1;
            for (x = x1; x < x2; x++)
            {
                RIA.rw0 = color;
            }
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
            if (++x >= 320)
                x = -320;
        }
        if (y_scroll)
        {
            xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, y);
            if (++y >= 180)
                y = -180;
        }
    }
}

void main()
{
    xreg_vga_canvas(2);
    clear();

    puts("You are standing on the bridge.");
    puts(">look");
    puts("A legion of two-dimensional lifeforms");
    puts("obscures your vision.");
    putchar('>');

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, true);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, true);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 320);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 180);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(3, 2, 0xFF00, 0, 0, 140);
    xreg_vga_mode(0, 1, 140, 180);

    box(10);
    scroll(true, true);
}
