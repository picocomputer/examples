#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>

// WORK IN PROGRESS

void task()
{
    const int16_t scale = 4; // mouse speed
    static int16_t x, y;
    static uint8_t mx, my;
    uint8_t m, color;
    uint16_t i;

    RIA.addr0 = 0xFFA1;
    m = RIA.rw0;
    if (mx != m)
    {
        x += (int8_t)(m - mx);
        mx = m;
        if (x < -scale)
            x = -scale;
        if (x > 318 * scale)
            x = 318 * scale;
    }

    RIA.addr0 = 0xFFA2;
    m = RIA.rw0;
    if (my != m)
    {
        y += (int8_t)(m - my);
        my = m;
        if (y < -scale)
            y = -scale;
        if (y > 238 * scale)
            y = 238 * scale;
    }

    xram1_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, x / scale);
    xram1_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, y / scale);

    RIA.addr0 = 0xFFA0;
    m = RIA.rw0;
    if (m & 1)
        color = 1;
    else if (m & 2)
        color = 2;
    else if (m & 4)
        color = 3;
    else
        return;

    RIA.step0 = 0;
    i = (y / scale + 1) * 160 + (x / scale + 1) / 2;
    if (x / scale & 1)
    {
        RIA.addr0 = i;
        RIA.rw0 = (RIA.rw0 & 0x0F) | color << 4;
    }
    else
    {
        RIA.addr0 = i;
        RIA.rw0 = (RIA.rw0 & 0xF0) | color;
    }
}

void pointer()
{
    // clang-format off
    const uint8_t data[100] = {
        16,16,16,16,16,16,16,0,0,0,
        16,255,255,255,255,255,16,0,0,0,
        16,255,255,255,255,16,0,0,0,0,
        16,255,255,255,255,16,0,0,0,0,
        16,255,255,255,255,255,16,0,0,0,
        16,255,16,16,255,255,255,16,0,0,
        16,16,0,0,16,255,255,255,16,0,
        0,0,0,0,0,16,255,255,255,16,
        0,0,0,0,0,0,16,255,16,0,
        0,0,0,0,0,0,0,16,0,0,
    };
    // clang-format on
    int i;
    RIA.addr0 = 0xFE00;
    RIA.step0 = 1;
    for (i = 0; i < 100; i++)
        RIA.rw0 = data[i];
}

void erase()
{
    unsigned i;
    RIA.addr0 = 0x0000;
    RIA.step0 = 1;
    for (i = 0; i < 320 * 240 / 2; i++)
        RIA.rw0 = 0;
}
void main()
{
    xreg_vga_canvas(1);
    xreg_ria_mouse(0xFFA0);

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 10);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 10);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0xFE00);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(0xFF10, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF10, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF10, vga_mode3_config_t, width_px, 320);
    xram0_struct_set(0xFF10, vga_mode3_config_t, height_px, 240);
    xram0_struct_set(0xFF10, vga_mode3_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF10, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(3, 2, 0xFF00, 2);
    // xreg_vga_mode(0, 1); // console
    xreg_vga_mode(3, 1, 0xFF10);

    pointer();
    erase();
    while (1)
        task();
}
