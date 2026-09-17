/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#ifndef XRAM_H
#define XRAM_H

#include <stddef.h>
#include <stdint.h>

#define PSG_CHANNELS 8

#define PSG_WAVE_SINE 0x00
#define PSG_WAVE_SQUARE 0x10
#define PSG_WAVE_SAWTOOTH 0x20
#define PSG_WAVE_TRIANGLE 0x30
#define PSG_WAVE_NOISE 0x40

#define PSG_GATE 0x01

#define PSG_FREQ_HZ(hz) ((hz) * 3u)
#define PSG_PAN(pan) ((uint8_t)((pan) * 2))

#define xreg_ria_psg(...) xreg(0, 1, 0, __VA_ARGS__)

typedef struct
{
    struct
    {
        uint16_t freq;
        uint8_t duty;
        uint8_t vol_attack;
        uint8_t vol_decay;
        uint8_t wave_release;
        uint8_t pan_gate;
        uint8_t reserved;
    } channel[PSG_CHANNELS];
} psg_t;

typedef struct
{
    psg_t psg;
} xram_layout_t;

#define XRAM_PSG offsetof(xram_layout_t, psg)

#endif
