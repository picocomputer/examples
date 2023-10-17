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

// Mouse speed divider
#define MOUSE_DIV 4

// Color Picker, do not change
#define PICKER_WIDTH 111
#define PICKER_HEIGHT 9

static uint8_t left_color = 15;
static uint8_t right_color = 14;
static uint8_t color;
static bool draw;
static bool drag;
int picker_x, picker_y;
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
    picker_x = x;
    picker_y = y;
    if (picker_x < 0)
        picker_x = 0;
    if (picker_x > CANVAS_WIDTH - PICKER_WIDTH)
        picker_x = CANVAS_WIDTH - PICKER_WIDTH;
    if (picker_y < 0)
        picker_y = 0;
    if (picker_y > CANVAS_HEIGHT - PICKER_HEIGHT)
        picker_y = CANVAS_HEIGHT - PICKER_HEIGHT;
    xram0_struct_set(0xFF10, vga_mode3_config_t, x_pos_px, picker_x);
    xram0_struct_set(0xFF10, vga_mode3_config_t, y_pos_px, picker_y);
}

void move(int x, int y)
{
    if (drag)
    {
        move_picker(x - drag_x, y - drag_y);
    }
    else if (draw)
    {
        RIA.step0 = 0;
        RIA.addr0 = y * 160 + x / 2;
        if (x & 1)
            RIA.rw0 = (RIA.rw0 & 0xF0) | color;
        else
            RIA.rw0 = (RIA.rw0 & 0x0F) | color << 4;
    }
}

// Returns -1 = draw, 0-15 = color, 16 = drag, 17 = erase, 18 = none
int picker_num(int x, int y)
{
    int num;
    x -= picker_x;
    y -= picker_y;
    if (x < 0 || x >= PICKER_WIDTH ||
        y < 0 || y >= PICKER_HEIGHT)
        return -1;
    if (x < 2 || x >= PICKER_WIDTH - 1 ||
        y < 2 || y >= PICKER_HEIGHT - 1)
        return 18;
    num = (x - 2) / 6;
    if (num == 0)
        num = 16;
    else if (num == 16)
        num = 0;
    return num;
}

void left_press(int x, int y)
{
    int num = picker_num(x, y);
    if (num < 0)
    {
        draw = true;
        color = left_color;
    }
    if (num >= 0 && num <= 15)
        left_color = num;
    if (num == 16)
    {
        drag = true;
        drag_x = x - picker_x;
        drag_y = y - picker_y;
    }
    if (num == 17)
        erase();
}

void left_release()
{
    draw = false;
    drag = false;
}

void right_press(int x, int y)
{
    int num = picker_num(x, y);
    if (num < 0)
    {
        draw = true;
        color = right_color;
    }
    if (num >= 0 && num <= 15)
        right_color = num;
}

void right_release()
{
    draw = false;
}

void mouse(unsigned addr)
{
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
        if (sx < -MOUSE_DIV)
            sx = -MOUSE_DIV;
        if (sx > (CANVAS_WIDTH - 2) * MOUSE_DIV)
            sx = (CANVAS_WIDTH - 2) * MOUSE_DIV;
    }

    RIA.addr0 = 0xFFA2;
    rw = RIA.rw0;
    if (my != rw)
    {
        sy += (int8_t)(rw - my);
        my = rw;
        if (sy < -MOUSE_DIV)
            sy = -MOUSE_DIV;
        if (sy > (CANVAS_HEIGHT - 2) * MOUSE_DIV)
            sy = (CANVAS_HEIGHT - 2) * MOUSE_DIV;
    }

    x = sx / MOUSE_DIV;
    y = sy / MOUSE_DIV;
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
        left_release();
    if (pressed & 2)
        right_press(x, y);
    if (released & 2)
        right_release();
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

    for (i = 1; i < 17; i++) // colors (black moved to end)
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

    xreg_vga_mode(3, 1, 0xFF00, 0);
    xreg_vga_mode(3, 2, 0xFF10, 1);
    xreg_vga_mode(3, 2, 0xFF20, 2);

    xreg_ria_mouse(0xFFA0);
    while (1)
        mouse(0xFF20);
}
