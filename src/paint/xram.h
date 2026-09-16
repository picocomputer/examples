/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stddef.h>
#include <stdint.h>

#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 240
#define PICKER_WIDTH 117
#define PICKER_HEIGHT 9
#define POINTER_SIZE 10

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
    } contact[8];
} tablet_t;

typedef struct
{
    uint8_t buttons;
    uint8_t x;
    uint8_t y;
    uint8_t wheel;
    uint8_t pan;
} mouse_t;

typedef struct
{
    vga_mode3_config_t canvas_config;
    vga_mode3_config_t picker_config;
    vga_mode3_config_t pointer_config;
    tablet_t tab;
    mouse_t mou;
    uint8_t mou_pad;
    uint8_t canvas[CANVAS_WIDTH / 2UL * CANVAS_HEIGHT];
    uint8_t picker[PICKER_WIDTH * PICKER_HEIGHT];
    uint8_t picker_pad;
    uint8_t pointer[POINTER_SIZE * POINTER_SIZE];
} xram_layout_t;

#define XRAM_CANVAS_CONFIG  offsetof(xram_layout_t, canvas_config)
#define XRAM_PICKER_CONFIG  offsetof(xram_layout_t, picker_config)
#define XRAM_POINTER_CONFIG offsetof(xram_layout_t, pointer_config)
#define XRAM_TAB_DATA       offsetof(xram_layout_t, tab)
#define XRAM_MOU_DATA       offsetof(xram_layout_t, mou)
#define XRAM_CANVAS_DATA    offsetof(xram_layout_t, canvas)
#define XRAM_PICKER_DATA    offsetof(xram_layout_t, picker)
#define XRAM_POINTER_DATA   offsetof(xram_layout_t, pointer)
