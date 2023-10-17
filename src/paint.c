#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>

static uint8_t color;
static bool left_down;
static bool right_down;

void erase()
{
    unsigned i;
    RIA.addr0 = 0x0000;
    RIA.step0 = 1;
    for (i = 0; i < 320 * 240 / 2 / 8; i++)
    {
        // unrolled for speed
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

void move(int16_t x, int16_t y)
{
    if (left_down || right_down)
    {
        RIA.step0 = 0;
        RIA.addr0 = y * 160 + x / 2;
        if (x & 1)
            RIA.rw0 = (RIA.rw0 & 0xF0) | color;
        else
            RIA.rw0 = (RIA.rw0 & 0x0F) | color << 4;
    }
}

void left_press(int16_t x, int16_t y)
{
    (void)x;
    (void)y;
    left_down = true;
    color = 1;
}

void left_release(int16_t x, int16_t y)
{
    (void)x;
    (void)y;
    left_down = false;
}

void right_press(int16_t x, int16_t y)
{
    (void)x;
    (void)y;
    right_down = true;
    color = 2;
}

void right_release(int16_t x, int16_t y)
{
    (void)x;
    (void)y;
    right_down = false;
}

void middle_press(int16_t x, int16_t y)
{
    (void)x;
    (void)y;
    erase();
}

void middle_release(int16_t x, int16_t y)
{
    (void)x;
    (void)y;
}

void mouse(unsigned addr)
{
    const int16_t ispeed = 4;
    static int16_t sx, sy;
    static uint8_t mb, mx, my;

    int16_t x, y;
    uint8_t rw, changed, pressed, released;

    RIA.addr0 = 0xFFA1;
    rw = RIA.rw0;
    if (mx != rw)
    {
        sx += (int8_t)(rw - mx);
        mx = rw;
        if (sx < -ispeed)
            sx = -ispeed;
        if (sx > 318 * ispeed)
            sx = 318 * ispeed;
    }

    RIA.addr0 = 0xFFA2;
    rw = RIA.rw0;
    if (my != rw)
    {
        sy += (int8_t)(rw - my);
        my = rw;
        if (sy < -ispeed)
            sy = -ispeed;
        if (sy > 238 * ispeed)
            sy = 238 * ispeed;
    }

    x = sx / ispeed;
    y = sy / ispeed;
    xram0_struct_set(addr, vga_mode3_config_t, x_pos_px, x);
    xram0_struct_set(addr, vga_mode3_config_t, y_pos_px, y);
    x++, y++;

    RIA.addr0 = 0xFFA0;
    rw = RIA.rw0;
    changed = mb ^ rw;
    pressed = rw & changed;
    released = mb & changed;
    mb = rw;
    if (pressed & 1)
        left_press(x, y);
    if (released & 1)
        left_release(x, y);
    if (pressed & 2)
        right_press(x, y);
    if (released & 2)
        right_release(x, y);
    if (pressed & 4)
        middle_press(x, y);
    if (released & 4)
        middle_release(x, y);
    move(x, y);
}

void pointer(unsigned addr)
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
    RIA.addr0 = addr;
    RIA.step0 = 1;
    for (i = 0; i < 100; i++)
        RIA.rw0 = data[i];
}

void main()
{
    xreg_vga_canvas(1);
    xreg_ria_mouse(0xFFA0);

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, 320);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, 240);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(0xFF10, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF10, vga_mode3_config_t, x_pos_px, 50);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF10, vga_mode3_config_t, width_px, 200);
    xram0_struct_set(0xFF10, vga_mode3_config_t, height_px, 9);
    xram0_struct_set(0xFF10, vga_mode3_config_t, xram_data_ptr, 0xA000);
    xram0_struct_set(0xFF10, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(0xFF20, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF20, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF20, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF20, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF20, vga_mode3_config_t, width_px, 10);
    xram0_struct_set(0xFF20, vga_mode3_config_t, height_px, 10);
    xram0_struct_set(0xFF20, vga_mode3_config_t, xram_data_ptr, 0xB000);
    xram0_struct_set(0xFF20, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(3, 1, 0xFF00, 0);
    // xreg_vga_mode(3, 2, 0xFF10, 1);
    xreg_vga_mode(3, 2, 0xFF20, 2);

    pointer(0xB000);
    erase();
    while (1)
        mouse(0xFF20);
}
