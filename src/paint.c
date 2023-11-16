/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>

// 320x180 and 320x240 supported
#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 240

// Mouse speed divider
#define MOUSE_DIV 4

// XRAM locations
#define CANVAS_STRUCT 0xFF00
#define PICKER_STRUCT 0xFF10
#define POINTER_STRUCT 0xFF20
#define CANVAS_DATA 0x0000
#define PICKER_DATA 0xA000
#define POINTER_DATA 0xB000
#define MOUSE_INPUT 0xFFA0

// Color Picker, do not change
#define PICKER_WIDTH 111
#define PICKER_HEIGHT 9

static uint8_t left_color;
static uint8_t right_color;
static uint8_t active_color;
static bool is_drawing;
static bool is_dragging;
static int picker_x, picker_y;
static int drag_x, drag_y;

static void erase_canvas(void)
{
    unsigned i;
    RIA.addr0 = CANVAS_DATA;
    RIA.step0 = 1;
    for (i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT / 2 / 16; i++)
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

static void draw_picker_box(uint8_t color, int x1, int y1, int x2, int y2)
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
        RIA.addr0 = PICKER_DATA + PICKER_WIDTH * y + x1;
        for (x = x1; x <= x2; x++)
        {
            RIA.rw0 = color;
        }
    }
}

static void draw_picker_color(uint8_t color)
{
    int x;
    uint8_t palette_color = color;
    // This swaps black to the end after white,
    // which is also the non-transparent black.
    if (!color)
        palette_color = 16;
    x = 2 + palette_color * 6;
    draw_picker_box(palette_color, x, 2, x + 4, 6);
    // Chop corner indicators
    if (left_color == color)
        draw_picker_box(240, x, 5, x + 1, 6);
    if (right_color == color)
        draw_picker_box(240, x + 3, 5, x + 4, 6);
    draw_picker_box(palette_color, x + 1, 5, x + 3, 5);
}

static void draw_picker(void)
{
    uint8_t i;
    draw_picker_box(250, 0, 0, 110, 8);   // border
    draw_picker_box(240, 1, 1, 109, 7);   // fill
    draw_picker_box(231, 2, 2, 6, 2);     // bar1
    draw_picker_box(231, 2, 4, 6, 4);     // bar2
    draw_picker_box(231, 2, 6, 6, 6);     // bar3
    draw_picker_box(231, 104, 2, 108, 6); // eraser border
    draw_picker_box(240, 105, 3, 107, 5); // eraser fill
    for (i = 0; i < 16; i++)
        draw_picker_color(i); // colors
}

static void move_picker(int x, int y)
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
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, x_pos_px, picker_x);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, y_pos_px, picker_y);
}

static void move(int x, int y)
{
    if (is_dragging)
    {
        move_picker(x - drag_x, y - drag_y);
    }
    else if (is_drawing)
    {
        RIA.step0 = 0;
        RIA.addr0 = y * 160 + x / 2;
        if (x & 1)
            RIA.rw0 = (RIA.rw0 & 0xF0) | active_color;
        else
            RIA.rw0 = (RIA.rw0 & 0x0F) | active_color << 4;
    }
}

// Returns -1 = draw, 0-15 = color, 16 = drag, 17 = erase, 18 = none
static int picker_num(int x, int y)
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

static void change_left_color(uint8_t color)
{
    uint8_t old_left = left_color;
    left_color = color;
    draw_picker_color(old_left);
    draw_picker_color(left_color);
}

static void change_right_color(uint8_t color)
{
    uint8_t old_right = right_color;
    right_color = color;
    draw_picker_color(old_right);
    draw_picker_color(right_color);
}

static void left_press(int x, int y)
{
    int num = picker_num(x, y);
    if (num < 0)
    {
        is_drawing = true;
        active_color = left_color;
    }
    if (num >= 0 && num <= 15)
        change_left_color(num);
    if (num == 16)
    {
        is_dragging = true;
        drag_x = x - picker_x;
        drag_y = y - picker_y;
    }
    if (num == 17)
        erase_canvas();
}

static void left_release(void)
{
    is_drawing = false;
    is_dragging = false;
}

static void right_press(int x, int y)
{
    int num = picker_num(x, y);
    if (num < 0)
    {
        is_drawing = true;
        active_color = right_color;
    }
    if (num >= 0 && num <= 15)
        change_right_color(num);
}

static void right_release(void)
{
    is_drawing = false;
}

static void mouse(void)
{
    static int sx, sy;
    static uint8_t mb, mx, my;

    int x, y;
    uint8_t rw, changed, pressed, released;

    RIA.addr0 = MOUSE_INPUT + 1;
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

    RIA.addr0 = MOUSE_INPUT + 2;
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
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, x_pos_px, x);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, y_pos_px, y);
    x++, y++;

    RIA.addr0 = MOUSE_INPUT + 0;
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

static void draw_pointer(void)
{
    // clang-format off
    const uint8_t data[100] = {
        16,16,16,16,16,16,16,0,0,0,16,255,255,255,255,255,16,0,0,0,
        16,255,255,255,255,16,0,0,0,0,16,255,255,255,255,16,0,0,0,0,
        16,255,255,255,255,255,16,0,0,0,16,255,16,16,255,255,255,16,0,0,
        16,16,0,0,16,255,255,255,16,0,0,0,0,0,0,16,255,255,255,16,
        0,0,0,0,0,0,16,255,16,0,0,0,0,0,0,0,0,16,0,0,
    };
    // clang-format on
    int i;
    RIA.addr0 = POINTER_DATA;
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

    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, width_px, CANVAS_WIDTH);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, height_px, CANVAS_HEIGHT);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, xram_data_ptr, CANVAS_DATA);
    xram0_struct_set(CANVAS_STRUCT, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, width_px, PICKER_WIDTH);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, height_px, PICKER_HEIGHT);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, xram_data_ptr, PICKER_DATA);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, width_px, 10);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, height_px, 10);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, xram_data_ptr, POINTER_DATA);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    erase_canvas();
    draw_picker();
    move_picker(104, 0);
    change_left_color(15);
    change_right_color(8);
    draw_pointer();

    xreg_vga_mode(3, 2, CANVAS_STRUCT, 0);
    xreg_vga_mode(3, 3, PICKER_STRUCT, 1);
    xreg_vga_mode(3, 3, POINTER_STRUCT, 2);

    xreg_ria_mouse(MOUSE_INPUT);
    while (1)
        mouse();
}
