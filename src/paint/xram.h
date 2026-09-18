/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#ifndef XRAM_H
#define XRAM_H

#include <rp6502.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 240
#define PICKER_WIDTH 117
#define PICKER_HEIGHT 9
#define POINTER_SIZE 10

#define xreg_vga_mode3(...) xreg(1, 0, 1, 3, __VA_ARGS__)

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_px;
    int16_t height_px;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
} mode3_config_t;

#define xreg_vga_canvas(...) xreg(1, 0, 0, __VA_ARGS__)

#define MOUSE_BUTTON_LEFT 0x01
#define MOUSE_BUTTON_RIGHT 0x02
#define MOUSE_BUTTON_MIDDLE 0x04
#define MOUSE_BUTTON_BACKWARD 0x08
#define MOUSE_BUTTON_FORWARD 0x10

#define xreg_ria_mouse(...) xreg(0, 0, 1, __VA_ARGS__)

typedef struct
{
    uint8_t buttons;
    uint8_t x;
    uint8_t y;
    uint8_t wheel;
    uint8_t pan;
    uint8_t pad;
} mouse_t;

#define TABLET_CONTACTS 8

#define TABLET_STATUS_HOST_CURSOR 0x01

#define TABLET_FLAG_LEFT 0x01
#define TABLET_FLAG_RIGHT 0x02
#define TABLET_FLAG_MIDDLE 0x04
#define TABLET_FLAG_BACKWARD 0x08
#define TABLET_FLAG_FORWARD 0x10
#define TABLET_FLAG_HOVER 0x80

#define TABLET_CURSOR_OFF 0
#define TABLET_CURSOR_ARROW 1
#define TABLET_CURSOR_CROSSHAIR 2
#define TABLET_CURSOR_IBEAM 3
#define TABLET_CURSOR_HAND 4
#define TABLET_CURSOR_RESIZE_EW 5
#define TABLET_CURSOR_RESIZE_NS 6

#define xreg_ria_tablet(...) xreg(0, 0, 3, __VA_ARGS__)

typedef struct
{
    uint8_t control;
    uint8_t status;
    uint8_t wheel;
    uint8_t pan;
    struct
    {
        uint8_t flags;
        uint8_t x0, x1, x2;
        uint8_t y0, y1;
    } contact[TABLET_CONTACTS];
} tablet_t;

typedef struct
{
    mode3_config_t picture_config;
    mode3_config_t picker_config;
    mode3_config_t pointer_config;
    tablet_t tab;
    mouse_t mou;
    uint8_t picture[CANVAS_WIDTH / 2UL * CANVAS_HEIGHT];
    uint8_t picker[PICKER_WIDTH * PICKER_HEIGHT];
    uint8_t picker_pad;
    uint8_t pointer[POINTER_SIZE * POINTER_SIZE];
} xram_layout_t;

#define XRAM_PICTURE_CONFIG offsetof(xram_layout_t, picture_config)
#define XRAM_PICKER_CONFIG  offsetof(xram_layout_t, picker_config)
#define XRAM_POINTER_CONFIG offsetof(xram_layout_t, pointer_config)
#define XRAM_TAB_DATA       offsetof(xram_layout_t, tab)
#define XRAM_MOU_DATA       offsetof(xram_layout_t, mou)
#define XRAM_PICTURE_DATA   offsetof(xram_layout_t, picture)
#define XRAM_PICKER_DATA    offsetof(xram_layout_t, picker)
#define XRAM_POINTER_DATA   offsetof(xram_layout_t, pointer)

#endif
