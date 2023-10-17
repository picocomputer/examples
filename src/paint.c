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

// 320x180 and 320x240 supported
#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 240

// Color Picker, do not change
#define PICKER_WIDTH 111
#define PICKER_HEIGHT 9

static uint8_t color;
static bool left_down;
static bool right_down;
static bool picker_drag;
int pick_x, pick_y;
int drag_x, drag_y;

void erase()
{
    unsigned i;
    RIA.addr0 = 0x0000;
    RIA.step0 = 1;
    for (i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT / 2 / 8; i++)
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

void move_picker(int x, int y)
{
    pick_x = x;
    pick_y = y;
    if (pick_x < 0)
        pick_x = 0;
    if (pick_x > CANVAS_WIDTH - PICKER_WIDTH)
        pick_x = CANVAS_WIDTH - PICKER_WIDTH;
    if (pick_y < 0)
        pick_y = 0;
    if (pick_y > CANVAS_HEIGHT - PICKER_HEIGHT)
        pick_y = CANVAS_HEIGHT - PICKER_HEIGHT;
    xram0_struct_set(0xFF10, vga_mode3_config_t, x_pos_px, pick_x);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_pos_px, pick_y);
}

void move(int x, int y)
{
    if (picker_drag)
    {
        move_picker(x - drag_x, y - drag_y);
    }
    else if (left_down || right_down)
    {
        RIA.step0 = 0;
        RIA.addr0 = y * 160 + x / 2;
        if (x & 1)
            RIA.rw0 = (RIA.rw0 & 0xF0) | color;
        else
            RIA.rw0 = (RIA.rw0 & 0x0F) | color << 4;
    }
}

void left_press(int x, int y)
{
    (void)x;
    (void)y;
    left_down = true;
    color = 1;

    if (x >= pick_x && x < pick_x + PICKER_WIDTH &&
        y >= pick_y && y < pick_y + PICKER_HEIGHT)
    {
        picker_drag = true;
        drag_x = x - pick_x;
        drag_y = y - pick_y;
    }
}

void left_release(int x, int y)
{
    (void)x;
    (void)y;
    left_down = false;
    picker_drag = false;
}

void right_press(int x, int y)
{
    (void)x;
    (void)y;
    right_down = true;
    color = 2;
}

void right_release(int x, int y)
{
    (void)x;
    (void)y;
    right_down = false;
}

void mouse(unsigned addr)
{
    const int ispeed = 4;
    static int sx, sy;
    static uint8_t mb, mx, my;

    int x, y;
    uint8_t rw, changed, pressed, released;

    RIA.addr0 = 0xFFA1;
    rw = RIA.rw0;
    if (mx != rw)
    {
        sx += (int8_t)(rw - mx);
        mx = rw;
        if (sx < -ispeed)
            sx = -ispeed;
        if (sx > (CANVAS_WIDTH - 2) * ispeed)
            sx = (CANVAS_WIDTH - 2) * ispeed;
    }

    RIA.addr0 = 0xFFA2;
    rw = RIA.rw0;
    if (my != rw)
    {
        sy += (int8_t)(rw - my);
        my = rw;
        if (sy < -ispeed)
            sy = -ispeed;
        if (sy > (CANVAS_HEIGHT - 2) * ispeed)
            sy = (CANVAS_HEIGHT - 2) * ispeed;
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
    move(x, y);
}

void picker_box(unsigned addr, uint8_t color, int x1, int y1, int x2, int y2)
{
    int x, y;
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
    RIA.step0 = 1;
    for (y = y1; y <= y2; y++)
    {
        RIA.addr0 = addr + PICKER_WIDTH * y + x1;
        for (x = x1; x <= x2; x++)
        {
            RIA.rw0 = color;
        }
    }
}

void picker(unsigned addr)
{
    uint8_t i;

    picker_box(addr, 250, 0, 0, 110, 8); // border
    picker_box(addr, 240, 1, 1, 109, 7); // fill

    picker_box(addr, 231, 2, 2, 6, 2); // bar1
    picker_box(addr, 231, 2, 4, 6, 4); // bar2
    picker_box(addr, 231, 2, 6, 6, 6); // bar3

    picker_box(addr, 231, 104, 2, 108, 6); // eraser border
    picker_box(addr, 240, 105, 3, 107, 5); // eraser fill

    for (i = 1; i < 17; i++) // colors
    {
        int x = 2 + i * 6;
        picker_box(addr, i, x, 2, x + 4, 6);
    }
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
#if CANVAS_HEIGHT == 240
    xreg_vga_canvas(1);
#elif CANVAS_HEIGHT == 180
    xreg_vga_canvas(2);
#else
#error invalid canvas height
#endif

    xram0_struct_set(0xFF00, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF00, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(0xFF00, vga_mode3_config_t, width_px, CANVAS_WIDTH);
    xram0_struct_set(0xFF00, vga_mode3_config_t, height_px, CANVAS_HEIGHT);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_data_ptr, 0x0000);
    xram0_struct_set(0xFF00, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(0xFF10, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF10, vga_mode3_config_t, width_px, PICKER_WIDTH);
    xram0_struct_set(0xFF10, vga_mode3_config_t, height_px, PICKER_HEIGHT);
    xram0_struct_set(0xFF10, vga_mode3_config_t, xram_data_ptr, 0xA000);
    xram0_struct_set(0xFF10, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(0xFF20, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(0xFF20, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(0xFF20, vga_mode3_config_t, width_px, 10);
    xram0_struct_set(0xFF20, vga_mode3_config_t, height_px, 10);
    xram0_struct_set(0xFF20, vga_mode3_config_t, xram_data_ptr, 0xB000);
    xram0_struct_set(0xFF20, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    erase();
    picker(0xA000);
    move_picker(104, 0);
    pointer(0xB000);

    // xreg_vga_mode(0, 0);

    xreg_vga_mode(3, 1, 0xFF00, 0);
    // xreg_vga_mode(3, 2, 0xFF10, 1);
    xreg_vga_mode(3, 2, 0xFF20, 2);

    xreg_ria_mouse(0xFFA0);
    while (1)
        mouse(0xFF20);
}
