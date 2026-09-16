/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#ifndef XRAM_H
#define XRAM_H

#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>

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
#define xreg_vga_mode(...) xreg(1, 0, 1, __VA_ARGS__)

typedef struct
{
    uint8_t keys[32];
} keyboard_t;

#define KEYBOARD_NO_KEY 0
#define KEYBOARD_NUM_LOCK 1
#define KEYBOARD_CAPS_LOCK 2
#define KEYBOARD_SCROLL_LOCK 3

#define KEYBOARD_PRESSED(keys, code) ((keys)[(code) >> 3] & (1 << ((code) & 7)))

#define xreg_ria_keyboard(...) xreg(0, 0, 0, __VA_ARGS__)

#endif
