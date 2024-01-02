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

#define bar_0 piano(e5, 2),  \
              wait(2),       \
              piano(ds5, 2), \
              wait(2)
#define bar_1 piano(e5, 2),  \
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
#define bar_2 piano(a4, 12), \
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
#define bar_3 piano(b4, 4),  \
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
#define bar_4 piano(c5, 4),  \
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
#define bar_5 piano(e5, 2),  \
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
#define bar_6 piano(a4, 4),  \
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
#define bar_7 piano(b4, 4),  \
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
#define bar_8_1 piano(a4, 8), \
                piano(a2, 4), \
                wait(2),      \
                piano(e3, 4), \
                wait(2),      \
                piano(a3, 4), \
                wait(4)
#define bar_8_2 piano(a4, 4), \
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
#define bar_9 piano(e5, 6),  \
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
#define bar_10 piano(d5, 6),  \
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
#define bar_11 piano(c5, 6),  \
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
#define bar_12 piano(b4, 8), \
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
#define bar_13 piano(e5, 8),  \
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
#define bar_14 piano(e5, 4),  \
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
#define bar_15 piano(e5, 2),  \
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
#define bar_16 piano(a4, 4),  \
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
#define bar_17 piano(b4, 4),  \
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
#define bar_18 piano(c5, 4),  \
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
#define bar_21 piano(b4, 4),  \
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
#define bar_22_1 piano(a4, 4), \
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
#define bar_22_2 piano(a4, 4),  \
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
#define bar_23 piano(f4, 2), \
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
               piano(f5, 3), \
               piano(c4, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(1),      \
               piano(e5, 1), \
               wait(1)
#define bar_24 piano(e5, 4),  \
               piano(d3, 2),  \
               wait(2),       \
               piano(as3, 2), \
               wait(2),       \
               piano(d5, 2),  \
               piano(d4, 2),  \
               wait(2),       \
               piano(as3, 2), \
               wait(2),       \
               piano(as5, 3), \
               piano(d4, 2),  \
               wait(2),       \
               piano(as3, 2), \
               wait(1),       \
               piano(a5, 1),  \
               wait(1)
#define bar_25 piano(a5, 2),  \
               piano(d3, 2),  \
               wait(2),       \
               piano(g5, 2),  \
               piano(e4, 2),  \
               wait(2),       \
               piano(f5, 2),  \
               piano(d3, 2),  \
               piano(e3, 2),  \
               piano(fs3, 2), \
               wait(2),       \
               piano(e5, 2),  \
               piano(e4, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               piano(d3, 2),  \
               piano(e3, 2),  \
               piano(fs3, 2), \
               wait(2),       \
               piano(c5, 2),  \
               piano(e4, 2),  \
               wait(2)
#define bar_26 piano(as4, 4), \
               piano(f3, 2),  \
               wait(2),       \
               piano(a3, 2),  \
               wait(2),       \
               piano(a4, 4),  \
               piano(c4, 2),  \
               wait(2),       \
               piano(as4, 4), \
               piano(a3, 2),  \
               wait(2),       \
               piano(a4, 1),  \
               piano(c4, 2),  \
               wait(1),       \
               piano(g4, 1),  \
               wait(1),       \
               piano(a4, 1),  \
               piano(a3, 2),  \
               wait(1),       \
               piano(as4, 1), \
               wait(1)
#define bar_27 piano(c5, 8),  \
               piano(f3, 2),  \
               wait(2),       \
               piano(a3, 2),  \
               wait(2),       \
               piano(c4, 2),  \
               wait(2),       \
               piano(a3, 2),  \
               wait(2),       \
               piano(d5, 2),  \
               piano(c4, 2),  \
               wait(2),       \
               piano(ds5, 2), \
               piano(a3, 2),  \
               wait(2)
#define bar_28 piano(e5, 6), \
               piano(e3, 2), \
               wait(2),      \
               piano(a3, 2), \
               wait(2),      \
               piano(c4, 2), \
               wait(2),      \
               piano(e5, 2), \
               piano(a3, 2), \
               wait(2),      \
               piano(f5, 2), \
               piano(d4, 2), \
               piano(d3, 2), \
               wait(2),      \
               piano(a4, 2), \
               piano(f3, 2), \
               wait(2)
#define bar_29 piano(c5, 8), \
               piano(g3, 2), \
               wait(2),      \
               piano(e4, 2), \
               wait(2),      \
               piano(g3, 2), \
               wait(2),      \
               piano(f4, 2), \
               wait(2),      \
               piano(d5, 3), \
               piano(g3, 2), \
               wait(2),      \
               piano(f4, 2), \
               wait(1),      \
               piano(b4, 1), \
               wait(1)
#define bar_30 piano(c5, 1), \
               piano(c4, 4), \
               piano(e4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(g4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(a4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(b4, 1), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c5, 1), \
               piano(e4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(d4, 2), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1)
#define bar_31 piano(e5, 1), \
               piano(g4, 4), \
               piano(e4, 4), \
               piano(c4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c6, 1), \
               wait(1),      \
               piano(b5, 1), \
               wait(1),      \
               piano(a5, 1), \
               piano(f3, 4), \
               piano(a3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f4, 1), \
               wait(1),      \
               piano(e5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(g3, 4), \
               piano(b3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f5, 1), \
               wait(1),      \
               piano(d5, 1), \
               wait(1)
#define bar_32 piano(c5, 1), \
               piano(c4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(g4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(a4, 1), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(b4, 1), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c5, 1), \
               piano(e4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(d4, 2), \
               piano(f4, 2), \
               piano(g4, 2), \
               wait(1),      \
               piano(g5, 1), \
               wait(1)
#define bar_33 piano(e5, 1), \
               piano(e4, 4), \
               piano(c4, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(c6, 1), \
               wait(1),      \
               piano(b5, 1), \
               wait(1),      \
               piano(a5, 1), \
               piano(f3, 4), \
               piano(a3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f4, 1), \
               wait(1),      \
               piano(e5, 1), \
               wait(1),      \
               piano(d5, 1), \
               piano(g3, 4), \
               piano(b3, 4), \
               wait(1),      \
               piano(g5, 1), \
               wait(1),      \
               piano(f5, 1), \
               wait(1),      \
               piano(d5, 1), \
               wait(1)
#define bar_34 piano(e5, 1),  \
               piano(b3, 4),  \
               piano(gs4, 4), \
               wait(1),       \
               piano(f5, 1),  \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(ds5, 1), \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(b4, 1),  \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(ds5, 1), \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(b4, 1),  \
               wait(1),       \
               piano(e5, 1),  \
               wait(1),       \
               piano(ds5, 1), \
               wait(1)
#define bar_35 piano(e5, 6), \
               wait(12)

static const uint8_t song[] = {
    wait(1),

    bar_0,
    bar_1,
    bar_2,
    bar_3,
    bar_4,
    bar_5,
    bar_6,
    bar_7,
    bar_8_1,

    bar_0,
    bar_1,
    bar_2,
    bar_3,
    bar_4,
    bar_5,
    bar_6,
    bar_7,
    bar_8_2,

    bar_9,
    bar_10,
    bar_11,
    bar_12,
    bar_13,
    bar_14,
    bar_15,
    bar_16,
    bar_17,
    bar_18,
    bar_15,
    bar_16,
    bar_21,
    bar_22_1,

    bar_9,
    bar_10,
    bar_11,
    bar_12,
    bar_13,
    bar_14,
    bar_15,
    bar_16,
    bar_17,
    bar_18,
    bar_15,
    bar_16,
    bar_21,
    bar_22_2,

    bar_23,
    bar_24,
    bar_25,
    bar_26,
    bar_27,
    bar_28,
    bar_29,
    bar_30,
    bar_31,
    bar_32,
    bar_33,
    bar_34,
    bar_35,

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
                        0x30,       // wave_release
                        0);         // pan
        break;
#ifndef NDEBUG
    default:
        // The instrumment you just added probably isn't
        // consuming the correct number of paramaters.
        puts("Unknown instrument.");
        exit(1);
#endif
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
