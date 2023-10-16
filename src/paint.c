#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>

// WORK IN PROGRESS

void task()
{
    const int16_t slow = 4; // use pow2
    static int16_t x, y;
    static uint8_t mx, my;

    RIA.addr0 = 0xFFA1;
    if (mx != RIA.rw0)
    {
        int8_t delta_x = RIA.rw0 - mx;
        mx = RIA.rw0;
        x += delta_x;
        if (x < -slow)
            x = -slow;
        if (x > 314 * slow * 2)
            x = 314 * slow * 2;
    }

    RIA.addr0 = 0xFFA2;
    if (my != RIA.rw0)
    {
        int8_t delta_y = RIA.rw0 - my;
        my = RIA.rw0;
        y += delta_y;
        if (y < -slow)
            y = -slow;
        if (y > 234 * slow * 2)
            y = 234 * slow * 2;
    }

    xram1_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, x / slow);
    xram1_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, y / slow);
}

void pointer()
{
    RIA.addr0 = 0xFE00;
    RIA.step0 = 1;

    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 16;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 16;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 16;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 16;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 16;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 255;
    RIA.rw0 = 16;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 16;
    RIA.rw0 = 255;
    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 16;
    RIA.rw0 = 16;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;

    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
    RIA.rw0 = 0;
}

void main()
{
    xreg_vga_canvas(3);
    xreg_ria_mouse(0xFFA0);

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 8);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 8);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0xFE00);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(3, 2, 0xFF00, 2);
    xreg_vga_mode(0);

    pointer();

    RIA.step0 = 0;
    while (1)
        task();
}
