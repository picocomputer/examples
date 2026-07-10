/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>

// 320x180 and 320x240 supported
#define CANVAS_WIDTH 320u
#define CANVAS_HEIGHT 240u

// XRAM locations
#define CANVAS_DATA 0x0000
#define PICKER_DATA 0xA000
#define POINTER_DATA 0xB000
#define CANVAS_STRUCT 0xFF00
#define PICKER_STRUCT 0xFF10
#define POINTER_STRUCT 0xFF20
#define TABLET_INPUT 0xFFA0

// Tablet report block (see the RIA tablet device docs).
#define TAB_STATUS (TABLET_INPUT + 0) // fw->ROM: bit0 host_cursor
#define TAB_STATUS_HOST_CURSOR 0x01
#define TAB_CONTROL (TABLET_INPUT + 1) // ROM->fw: cursor shape
#define TAB_CONTACT0 (TABLET_INPUT + 2)
// Within a contact: flags, x0, x1, x2, y0, y1
#define TAB_CURSOR_OFF 0
#define TAB_CURSOR_CROSSHAIR 2

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
static int line_x, line_y;

static int mouse_pos_x;
static int mouse_pos_y;
static bool host_draws; // the host renders the pointer; we hide our own sprite

// Decode one axis of the tablet's unary window encoding. The three (or two) bytes
// carry 1..255 for exactly one active window; first non-zero wins. If all are
// zero (a coordinate straddling a window edge was caught mid-update) the caller
// keeps the previous value.
static bool decode_x(uint8_t b0, uint8_t b1, uint8_t b2, int *out)
{
    if (b0)
        *out = b0 - 1;
    else if (b1)
        *out = b1 + 254;
    else if (b2)
        *out = b2 + 509;
    else
        return false;
    return true;
}

static bool decode_y(uint8_t b0, uint8_t b1, int *out)
{
    if (b0)
        *out = b0 - 1;
    else if (b1)
        *out = b1 + 254;
    else
        return false;
    return true;
}

// Read contact 0 (the mouse-compatible primary). Returns the button bits (low 3)
// and updates mouse_pos_x/y. X > 639 marks an inactive contact (pointer gone).
static uint8_t read_tablet(void)
{
    uint8_t flags, x0, x1, x2, y0, y1;
    int x, y, tries;

    for (tries = 0; tries < 2; ++tries)
    {
        RIA.addr0 = TAB_CONTACT0;
        RIA.step0 = 1;
        flags = RIA.rw0;
        x0 = RIA.rw0;
        x1 = RIA.rw0;
        x2 = RIA.rw0;
        y0 = RIA.rw0;
        y1 = RIA.rw0;
        if (x0 || x1 || x2) // coherent X sample
            break;
    }

    if (!decode_x(x0, x1, x2, &x))
        return 0; // all zero after a retry: leave the cursor where it was
    if (x > 639)
        return 0; // inactive contact (pointer left the canvas)
    if (!decode_y(y0, y1, &y))
        y = mouse_pos_y;

    mouse_pos_x = x;
    mouse_pos_y = y;
    return flags & 0x07;
}

static void tablet_init(void)
{
    xreg_ria_tablet(TABLET_INPUT);
}

static void erase_canvas(void)
{
    unsigned i;
    RIA.addr0 = CANVAS_DATA;
    RIA.step0 = 1;
    for (i = 0; i < CANVAS_WIDTH / 2 * CANVAS_HEIGHT; i++)
        RIA.rw0 = 0;
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

static void draw_pixel(int x, int y)
{
    RIA.step0 = 0;
    RIA.addr0 = y * 160 + x / 2;
    if (x & 1)
        RIA.rw0 = (RIA.rw0 & 0xF0) | active_color;
    else
        RIA.rw0 = (RIA.rw0 & 0x0F) | active_color << 4;
}

static void move(int x, int y)
{
    if (is_dragging)
    {
        move_picker(x - drag_x, y - drag_y);
    }
    else if (is_drawing)
    {
        // Bresenham's line algorithm
        int dx = x - line_x;
        int dy = y - line_y;
        int ax = dx < 0 ? -dx : dx;
        int ay = dy < 0 ? -dy : dy;
        int sx = dx < 0 ? -1 : 1;
        int sy = dy < 0 ? -1 : 1;
        int cx = line_x;
        int cy = line_y;
        if (ax > ay)
        {
            int d = 2 * ay - ax;
            while (cx != x)
            {
                draw_pixel(cx, cy);
                if (d > 0)
                {
                    cy += sy;
                    d -= 2 * ax;
                }
                d += 2 * ay;
                cx += sx;
            }
        }
        else
        {
            int d = 2 * ax - ay;
            while (cy != y)
            {
                draw_pixel(cx, cy);
                if (d > 0)
                {
                    cx += sx;
                    d -= 2 * ay;
                }
                d += 2 * ax;
                cy += sy;
            }
        }
        draw_pixel(x, y);
        line_x = x;
        line_y = y;
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
        line_x = x;
        line_y = y;
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
        line_x = x;
        line_y = y;
    }
    if (num >= 0 && num <= 15)
        change_right_color(num);
}

static void right_release(void)
{
    is_drawing = false;
}

static void tablet(void)
{
    static uint8_t mb;
    static uint8_t last_host = 0xFF;
    int x, y;
    uint8_t status, buttons, changed, pressed, released;

    // Let the host draw the pointer when it can (desktop/web with a mouse); draw
    // our own sprite only when it can't (real hardware, or a touchscreen).
    RIA.addr0 = TAB_STATUS;
    status = RIA.rw0 & TAB_STATUS_HOST_CURSOR;
    if (status != last_host)
    {
        last_host = status;
        host_draws = status != 0;
        RIA.addr0 = TAB_CONTROL;
        RIA.rw0 = host_draws ? TAB_CURSOR_CROSSHAIR : TAB_CURSOR_OFF;
        if (host_draws) // park our sprite off-canvas; the host draws the pointer
            xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, x_pos_px, CANVAS_WIDTH);
    }

    buttons = read_tablet();
    x = mouse_pos_x;
    y = mouse_pos_y;

    if (!host_draws)
    {
        xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, x_pos_px, x - 1);
        xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, y_pos_px, y - 1);
    }

    // Dispatch button events
    changed = mb ^ buttons;
    pressed = buttons & changed;
    released = mb & changed;
    mb = buttons;
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

    tablet_init();
    while (1)
        tablet();
}
