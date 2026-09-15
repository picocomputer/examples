/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

// A paint program for the tablet, or for the mouse when started as paint -m.

#include <rp6502.h>
#include <6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Up to 512 bytes are needed for argv (one xstack size).
// Applications must opt-in to argc/argv by providing this memory.
void *__fastcall__ __argv_mem(size_t size) { return malloc(size); }

#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 240
#define PICKER_WIDTH 111
#define PICKER_HEIGHT 9
#define POINTER_SIZE 10

// XRAM locations
#define CANVAS_DATA 0x0000
#define PICKER_DATA 0xA000
#define POINTER_DATA 0xB000
#define CANVAS_STRUCT 0xFF00
#define PICKER_STRUCT 0xFF10
#define POINTER_STRUCT 0xFF20
#define INPUT_DATA 0xFFA0

// Colors from the built-in 256 color palette
#define WHITE 231
#define DARK_GRAY 240
#define LIGHT_GRAY 250

// What the palette has under the pointer, when it isn't a color 0-15
#define PICK_CANVAS -1
#define PICK_GRIP 16
#define PICK_ERASER 17
#define PICK_BORDER 18

#define LEFT 0
#define RIGHT 1

static uint8_t color[2];
static uint8_t draw_color;
static bool is_drawing;
static bool is_dragging;
static int picker_x, picker_y;
static int drag_x, drag_y;
static int line_x, line_y;

static int clamp(int value, int low, int high)
{
    if (value < low)
        return low;
    if (value > high)
        return high;
    return value;
}

static void setup_bitmap(unsigned config, int width, int height, unsigned data)
{
    xram0_struct_set(config, vga_mode3_config_t, x_wrap, false);
    xram0_struct_set(config, vga_mode3_config_t, y_wrap, false);
    xram0_struct_set(config, vga_mode3_config_t, x_pos_px, 0);
    xram0_struct_set(config, vga_mode3_config_t, y_pos_px, 0);
    xram0_struct_set(config, vga_mode3_config_t, width_px, width);
    xram0_struct_set(config, vga_mode3_config_t, height_px, height);
    xram0_struct_set(config, vga_mode3_config_t, xram_data_ptr, data);
    xram0_struct_set(config, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);
}

// The point of the arrow is one pixel in from the corner of its image.
static void move_pointer(int x, int y)
{
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, x_pos_px, x - 1);
    xram0_struct_set(POINTER_STRUCT, vga_mode3_config_t, y_pos_px, y - 1);
}

static void draw_pointer(void)
{
    // clang-format off
    static const uint8_t image[POINTER_SIZE * POINTER_SIZE] = {
        16,16,16,16,16,16,16,0,0,0,16,255,255,255,255,255,16,0,0,0,
        16,255,255,255,255,16,0,0,0,0,16,255,255,255,255,16,0,0,0,0,
        16,255,255,255,255,255,16,0,0,0,16,255,16,16,255,255,255,16,0,0,
        16,16,0,0,16,255,255,255,16,0,0,0,0,0,0,16,255,255,255,16,
        0,0,0,0,0,0,16,255,16,0,0,0,0,0,0,0,0,16,0,0,
    };
    // clang-format on
    unsigned i;
    RIA.addr0 = POINTER_DATA;
    RIA.step0 = 1;
    for (i = 0; i < sizeof(image); i++)
        RIA.rw0 = image[i];
}

// ---------------------------------------------------------------------------
// Mouse
//
// The mouse reports relative motion as counters. The RIA docs recommend
// reading them at 125 Hz or faster, so a VIA timer interrupt keeps the
// position. On a 320 pixel wide canvas, two counts move one pixel.

#define MOUSE_DIV 2

static uint8_t mouse_irq_stack[32];
static uint8_t mouse_last_x, mouse_last_y;
static int mouse_x, mouse_y;

static unsigned char mouse_irq(void)
{
    static int raw_x, raw_y;
    uint16_t save_addr0 = RIA.addr0;
    uint8_t save_step0 = RIA.step0;
    uint8_t count;

    VIA.ifr = 0x40; // acknowledge timer 1

    RIA.addr0 = INPUT_DATA + 1;
    RIA.step0 = 1;
    count = RIA.rw0;
    raw_x += (int8_t)(count - mouse_last_x);
    mouse_last_x = count;
    count = RIA.rw0;
    raw_y += (int8_t)(count - mouse_last_y);
    mouse_last_y = count;

    raw_x = clamp(raw_x, 0, (CANVAS_WIDTH - 1) * MOUSE_DIV);
    raw_y = clamp(raw_y, 0, (CANVAS_HEIGHT - 1) * MOUSE_DIV);
    mouse_x = raw_x / MOUSE_DIV;
    mouse_y = raw_y / MOUSE_DIV;
    move_pointer(mouse_x, mouse_y);

    // The main loop was using RW0 when this interrupt arrived.
    RIA.addr0 = save_addr0;
    RIA.step0 = save_step0;
    return IRQ_HANDLED;
}

static void mouse_init(void)
{
    // Timer 1 repeats every period + 2 cycles, and PHI2 runs kHz * 8 cycles
    // in 8 ms.
    unsigned period = ria_attr_get(RIA_ATTR_PHI2_KHZ) * 8 - 2;

    xreg_ria_mouse(INPUT_DATA);
    RIA.addr0 = INPUT_DATA + 1;
    RIA.step0 = 1;
    mouse_last_x = RIA.rw0;
    mouse_last_y = RIA.rw0;

    set_irq(mouse_irq, &mouse_irq_stack, sizeof(mouse_irq_stack));
    VIA.t1l_lo = period & 0xFF;
    VIA.t1l_hi = period >> 8;
    VIA.t1_lo = period & 0xFF;
    VIA.t1_hi = period >> 8;
    VIA.acr = 0x40; // timer 1 free running
    VIA.ier = 0xC0; // timer 1 interrupt on
}

static uint8_t mouse_read(int *x, int *y)
{
    SEI();
    *x = mouse_x;
    *y = mouse_y;
    CLI();
    RIA.addr0 = INPUT_DATA;
    return RIA.rw0 & 0x03;
}

// ---------------------------------------------------------------------------
// Tablet
//
// The tablet reports the pointer as a canvas position. Each axis is split into
// one-byte windows, and only the window holding the value is non-zero. When
// the host can draw a cursor, as the emulator can for a mouse, the program
// hides its own pointer and asks for a crosshair.

#define TABLET_CONTROL (INPUT_DATA + 0)
#define TABLET_STATUS (INPUT_DATA + 1)
#define TABLET_CONTACT (INPUT_DATA + 4)
#define TABLET_HOST_CURSOR 0x01
#define CURSOR_OFF 0
#define CURSOR_CROSSHAIR 2

static int tablet_x, tablet_y;
static bool host_cursor;

static void tablet_init(void)
{
    xreg_ria_tablet(INPUT_DATA);
}

static uint8_t tablet_read(int *x, int *y)
{
    uint8_t flags, x0, x1, x2, y0, y1;
    bool offered;
    int tries;

    RIA.addr0 = TABLET_STATUS;
    offered = RIA.rw0 & TABLET_HOST_CURSOR;
    if (offered != host_cursor)
    {
        host_cursor = offered;
        RIA.addr0 = TABLET_CONTROL;
        RIA.rw0 = host_cursor ? CURSOR_CROSSHAIR : CURSOR_OFF;
    }

    // A read that lands while a value crosses into the next window can find
    // every window zero, so the contact is read a second time.
    for (tries = 0; tries < 2; tries++)
    {
        RIA.addr0 = TABLET_CONTACT;
        RIA.step0 = 1;
        flags = RIA.rw0;
        x0 = RIA.rw0;
        x1 = RIA.rw0;
        x2 = RIA.rw0;
        y0 = RIA.rw0;
        y1 = RIA.rw0;
        if ((x0 | x1 | x2) && (y0 | y1))
            break;
    }

    // With no window set, the position stays where it was.
    if (x0)
        tablet_x = x0 - 1;
    else if (x1)
        tablet_x = x1 + 254;
    else if (x2)
        tablet_x = x2 + 509;
    if (y0)
        tablet_y = y0 - 1;
    else if (y1)
        tablet_y = y1 + 254;

    if (host_cursor)
        move_pointer(CANVAS_WIDTH + 1, 0);
    else
        move_pointer(tablet_x, tablet_y);

    *x = tablet_x;
    *y = tablet_y;
    return flags & 0x03;
}

// ---------------------------------------------------------------------------
// Canvas

static void erase_canvas(void)
{
    unsigned i;
    RIA.addr0 = CANVAS_DATA;
    RIA.step0 = 1;
    for (i = 0; i < CANVAS_WIDTH / 2 * (unsigned)CANVAS_HEIGHT; i++)
        RIA.rw0 = 0;
}

// The canvas has four bits per pixel, so one byte holds two pixels.
static void draw_pixel(int x, int y)
{
    uint8_t pair;
    RIA.step0 = 0;
    RIA.addr0 = CANVAS_DATA + (unsigned)y * (CANVAS_WIDTH / 2) + x / 2;
    pair = RIA.rw0;
    if (x & 1)
        RIA.rw0 = (pair & 0xF0) | draw_color;
    else
        RIA.rw0 = (pair & 0x0F) | draw_color << 4;
}

// Bresenham's line algorithm
static void draw_line(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int err2;
    while (true)
    {
        draw_pixel(x0, y0);
        if (x0 == x1 && y0 == y1)
            break;
        err2 = 2 * err;
        if (err2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (err2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// ---------------------------------------------------------------------------
// Palette
//
// The palette is a small bitmap that floats over the canvas. From left to
// right it holds a grip for dragging it, the sixteen colors, and an eraser.

static void draw_picker_box(uint8_t shade, int x1, int y1, int x2, int y2)
{
    int x, y;
    RIA.step0 = 1;
    for (y = y1; y <= y2; y++)
    {
        RIA.addr0 = PICKER_DATA + PICKER_WIDTH * y + x1;
        for (x = x1; x <= x2; x++)
            RIA.rw0 = shade;
    }
}

// Palette index 0 is transparent, so black is drawn as the opaque black at
// index 16 and placed after white. A notch in the bottom corner marks the
// color held by that button.
static void draw_picker_color(uint8_t c)
{
    uint8_t shade = c ? c : 16;
    int x = 2 + shade * 6;
    draw_picker_box(shade, x, 2, x + 4, 6);
    if (color[LEFT] == c)
        draw_picker_box(DARK_GRAY, x, 5, x + 1, 6);
    if (color[RIGHT] == c)
        draw_picker_box(DARK_GRAY, x + 3, 5, x + 4, 6);
    draw_picker_box(shade, x + 1, 5, x + 3, 5);
}

static void draw_picker(void)
{
    uint8_t c;
    draw_picker_box(LIGHT_GRAY, 0, 0, PICKER_WIDTH - 1, PICKER_HEIGHT - 1);
    draw_picker_box(DARK_GRAY, 1, 1, PICKER_WIDTH - 2, PICKER_HEIGHT - 2);
    draw_picker_box(WHITE, 2, 2, 6, 2); // grip
    draw_picker_box(WHITE, 2, 4, 6, 4);
    draw_picker_box(WHITE, 2, 6, 6, 6);
    draw_picker_box(WHITE, 104, 2, 108, 6); // eraser
    draw_picker_box(DARK_GRAY, 105, 3, 107, 5);
    for (c = 0; c < 16; c++)
        draw_picker_color(c);
}

static void move_picker(int x, int y)
{
    picker_x = clamp(x, 0, CANVAS_WIDTH - PICKER_WIDTH);
    picker_y = clamp(y, 0, CANVAS_HEIGHT - PICKER_HEIGHT);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, x_pos_px, picker_x);
    xram0_struct_set(PICKER_STRUCT, vga_mode3_config_t, y_pos_px, picker_y);
}

static int picker_pick(int x, int y)
{
    int slot;
    x -= picker_x;
    y -= picker_y;
    if (x < 0 || x >= PICKER_WIDTH || y < 0 || y >= PICKER_HEIGHT)
        return PICK_CANVAS;
    if (x < 2 || x >= PICKER_WIDTH - 1 || y < 2 || y >= PICKER_HEIGHT - 1)
        return PICK_BORDER;
    slot = (x - 2) / 6;
    if (slot == 0)
        return PICK_GRIP;
    if (slot == 16)
        return 0;
    if (slot == 17)
        return PICK_ERASER;
    return slot;
}

static void set_color(int button, uint8_t c)
{
    uint8_t old = color[button];
    color[button] = c;
    draw_picker_color(old);
    draw_picker_color(c);
}

// ---------------------------------------------------------------------------
// Painting

static void press(int button, int x, int y)
{
    int pick = picker_pick(x, y);
    if (pick == PICK_CANVAS)
    {
        is_drawing = true;
        draw_color = color[button];
        line_x = x;
        line_y = y;
    }
    else if (pick < 16)
        set_color(button, pick);
    else if (pick == PICK_GRIP)
    {
        is_dragging = true;
        drag_x = x - picker_x;
        drag_y = y - picker_y;
    }
    else if (pick == PICK_ERASER)
        erase_canvas();
}

static void release(void)
{
    is_drawing = false;
    is_dragging = false;
}

static void move(int x, int y)
{
    if (is_dragging)
        move_picker(x - drag_x, y - drag_y);
    else if (is_drawing)
    {
        draw_line(line_x, line_y, x, y);
        line_x = x;
        line_y = y;
    }
}

int main(int argc, char *argv[])
{
    bool use_mouse = argc == 2 && strcmp(argv[1], "-m") == 0;
    uint8_t buttons, held = 0, pressed, released;
    int x, y;

    free(argv);

    xreg_vga_canvas(1); // 320x240
    setup_bitmap(CANVAS_STRUCT, CANVAS_WIDTH, CANVAS_HEIGHT, CANVAS_DATA);
    setup_bitmap(PICKER_STRUCT, PICKER_WIDTH, PICKER_HEIGHT, PICKER_DATA);
    setup_bitmap(POINTER_STRUCT, POINTER_SIZE, POINTER_SIZE, POINTER_DATA);

    erase_canvas();
    draw_picker();
    move_picker(104, 0);
    set_color(LEFT, 15);
    set_color(RIGHT, 8);
    draw_pointer();

    xreg_vga_mode(3, 2, CANVAS_STRUCT, 0);  // 4 bits per pixel, plane 0
    xreg_vga_mode(3, 3, PICKER_STRUCT, 1);  // 8 bits per pixel, plane 1
    xreg_vga_mode(3, 3, POINTER_STRUCT, 2); // 8 bits per pixel, plane 2

    if (use_mouse)
        mouse_init();
    else
        tablet_init();

    while (true)
    {
        buttons = use_mouse ? mouse_read(&x, &y) : tablet_read(&x, &y);
        pressed = buttons & ~held;
        released = held & ~buttons;
        held = buttons;
        if (pressed & 1)
            press(LEFT, x, y);
        if (pressed & 2)
            press(RIGHT, x, y);
        if (released)
            release();
        move(x, y);
    }
}
