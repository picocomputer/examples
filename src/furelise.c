/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ezpsg.h"
#include <rp6502.h>
#include <stdlib.h>
#include <stdio.h>

#define wait(duration) (duration)
#define piano(note, duration) (-1), (note), (duration)
#define end() (0)

#define bar_1_1 piano(e5, 2),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2)
#define bar_1_2 piano(e5, 2),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2),       \
                piano(e5, 2),  \
                wait(2),       \
                piano(b4, 2),  \
                wait(2),       \
                piano(d5, 2),  \
                wait(2),       \
                piano(c5, 2),  \
                wait(2)
#define bar_1_3 piano(a4, 12), \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(c4, 6),  \
                wait(2),       \
                piano(e4, 4),  \
                wait(2),       \
                piano(a4, 2),  \
                wait(2)
#define bar_1_4 piano(b4, 4),  \
                piano(e2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(gs3, 8), \
                wait(2),       \
                piano(e4, 6),  \
                wait(2),       \
                piano(gs4, 4), \
                wait(2),       \
                piano(b4, 2),  \
                wait(2)
#define bar_1_5 piano(c5, 4),  \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(e4, 6),  \
                wait(2),       \
                piano(e5, 4),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2)
#define bar_1_6 piano(e5, 2),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2),       \
                piano(e5, 2),  \
                wait(2),       \
                piano(b4, 2),  \
                wait(2),       \
                piano(d5, 2),  \
                wait(2),       \
                piano(c5, 2),  \
                wait(2)
#define bar_1_7 piano(a4, 4),  \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(c4, 6),  \
                wait(2),       \
                piano(e4, 4),  \
                wait(2),       \
                piano(a4, 2),  \
                wait(2)
#define bar_2_1 piano(b4, 4),  \
                piano(e2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(gs3, 8), \
                wait(2),       \
                piano(d4, 6),  \
                wait(2),       \
                piano(c5, 4),  \
                wait(2),       \
                piano(b4, 2),  \
                wait(2)
#define bar_2_2 piano(a4, 8), \
                piano(a2, 2), \
                wait(2),      \
                piano(e3, 2), \
                wait(2),      \
                piano(a3, 2), \
                wait(4)
#define bar_2_3 piano(a4, 4), \
                piano(a2, 2), \
                wait(2),      \
                piano(e3, 2), \
                wait(2),      \
                piano(a3, 2), \
                wait(2),      \
                piano(b4, 2), \
                wait(2),      \
                piano(c5, 2), \
                wait(2),      \
                piano(d5, 2), \
                wait(2)
#define bar_2_4 piano(e5, 6),  \
                piano(c3, 12), \
                wait(2),       \
                piano(g3, 10), \
                wait(2),       \
                piano(c4, 8),  \
                wait(2),       \
                piano(g4, 6),  \
                wait(2),       \
                piano(f5, 4),  \
                wait(2),       \
                piano(e5, 2),  \
                wait(2)
#define bar_2_5 piano(d5, 6),  \
                piano(g2, 12), \
                wait(2),       \
                piano(g3, 10), \
                wait(2),       \
                piano(b3, 8),  \
                wait(2),       \
                piano(f4, 6),  \
                wait(2),       \
                piano(e5, 4),  \
                wait(2),       \
                piano(d5, 2),  \
                wait(2)
#define bar_2_6 piano(c5, 6),  \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(e4, 8),  \
                wait(2),       \
                piano(d5, 8),  \
                wait(2),       \
                piano(c5, 8),  \
                wait(2)
#define bar_2_7 piano(b4, 8), \
                piano(e2, 8), \
                wait(2),      \
                piano(e3, 8), \
                wait(2),      \
                piano(e4, 8), \
                wait(2),      \
                piano(e4, 8), \
                wait(2),      \
                piano(e5, 8), \
                wait(2),      \
                piano(e4, 8), \
                wait(2)
#define bar_3_1 piano(e5, 8),  \
                wait(2),       \
                piano(e5, 8),  \
                wait(2),       \
                piano(e6, 8),  \
                wait(2),       \
                piano(ds5, 6), \
                wait(2),       \
                piano(e5, 4),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2)
#define bar_3_2 piano(e5, 4),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2),       \
                piano(e5, 2),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2),       \
                piano(e5, 2),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2)
#define bar_3_3 piano(e5, 2),  \
                wait(2),       \
                piano(ds5, 2), \
                wait(2),       \
                piano(e5, 2),  \
                wait(2),       \
                piano(b4, 2),  \
                wait(2),       \
                piano(d5, 2),  \
                wait(2),       \
                piano(c5, 2),  \
                wait(2)
#define bar_3_4 piano(a4, 4),  \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(c4, 8),  \
                wait(2),       \
                piano(e4, 8),  \
                wait(2),       \
                piano(a4, 8),  \
                wait(2)
#define bar_3_5 piano(b4, 4),  \
                piano(e2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(gs3, 8), \
                wait(2),       \
                piano(e4, 8),  \
                wait(2),       \
                piano(gs4, 8), \
                wait(2),       \
                piano(b4, 8),  \
                wait(2)
#define bar_3_6 piano(c5, 4),  \
                piano(a2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(a3, 8),  \
                wait(2),       \
                piano(e4, 8),  \
                wait(2),       \
                piano(e5, 8),  \
                wait(2),       \
                piano(ds5, 8), \
                wait(2)
#define bar_4_3 piano(b4, 4),  \
                piano(e2, 12), \
                wait(2),       \
                piano(e3, 10), \
                wait(2),       \
                piano(gs3, 8), \
                wait(2),       \
                piano(d4, 8),  \
                wait(2),       \
                piano(c5, 8),  \
                wait(2),       \
                piano(b4, 8),  \
                wait(2)
#define bar_4_4 piano(a4, 4), \
                piano(a2, 2), \
                wait(2),      \
                piano(e3, 2), \
                wait(2),      \
                piano(a3, 2), \
                wait(2),      \
                piano(b4, 2), \
                wait(2),      \
                piano(c5, 2), \
                wait(2),      \
                piano(d5, 2), \
                wait(2)
#define bar_4_5 piano(a4, 4),  \
                piano(a2, 2),  \
                wait(2),       \
                piano(e3, 2),  \
                wait(2),       \
                piano(a3, 2),  \
                wait(2),       \
                piano(as3, 2), \
                piano(c4, 2),  \
                piano(e4, 2),  \
                piano(c5, 2),  \
                wait(2),       \
                piano(a3, 2),  \
                piano(c4, 2),  \
                piano(f4, 2),  \
                piano(c5, 2),  \
                wait(2),       \
                piano(g3, 2),  \
                piano(as3, 2), \
                piano(c4, 2),  \
                piano(e4, 2),  \
                piano(g4, 2),  \
                piano(c5, 2),  \
                wait(2)
#define bar_4_6 piano(f4, 2), \
                piano(a4, 2), \
                piano(c5, 8), \
                piano(f3, 2), \
                wait(2),      \
                piano(a3, 2), \
                wait(2),      \
                piano(c4, 2), \
                wait(2),      \
                piano(a3, 2), \
                wait(2),      \
                piano(c4, 2), \
                piano(f5, 3), \
                wait(2),      \
                piano(a3, 2), \
                wait(1),      \
                piano(e5, 1), \
                wait(1)

uint8_t song[] = {
    wait(1),

    bar_1_1,
    bar_1_2,
    bar_1_3,
    bar_1_4,
    bar_1_5,
    bar_1_6,
    bar_1_7,
    bar_2_1,
    bar_2_2,

    bar_1_1,
    bar_1_2,
    bar_1_3,
    bar_1_4,
    bar_1_5,
    bar_1_6,
    bar_1_7,
    bar_2_1,
    bar_2_3,

    bar_2_4,
    bar_2_5,
    bar_2_6,
    bar_2_7,
    bar_3_1,
    bar_3_2,
    bar_3_3,
    bar_3_4,
    bar_3_5,
    bar_3_6,
    bar_3_3,
    bar_3_4,
    bar_4_3,
    bar_4_4,

    bar_2_4,
    bar_2_5,
    bar_2_6,
    bar_2_7,
    bar_3_1,
    bar_3_2,
    bar_3_3,
    bar_3_4,
    bar_3_5,
    bar_3_6,
    bar_3_3,
    bar_3_4,
    bar_4_3,
    bar_4_5,

    bar_4_6,

    end()};

void ezpsg_instruments(const uint8_t **data)
{
    switch ((int8_t) * (*data)++) // instrument
    {
    case -1:                        // piano
        ezpsg_play_note(*(*data)++, // note
                        *(*data)++, // duration
                        48000u,     // duty
                        0x11,       // vol_attack
                        0xF9,       // vol_decay
                        0x31,       // wave_release
                        0);         // pan
        break;
    default:
        puts("Bad instrument.");
        exit(1);
    }
}

void main(void)
{
    uint8_t v = RIA.vsync;

    ezpsg_init(0xFF00);
    ezpsg_play_song(song);

    while (true)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;
        ezpsg_tick(5);
        if (!ezpsg_playing())
            break;
    }
}
