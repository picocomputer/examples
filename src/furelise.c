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

#define bar_1_1 piano(e5, 1),  \
                wait(1),       \
                piano(ds5, 1), \
                wait(1)
#define bar_1_2 piano(e5, 1),  \
                wait(1),       \
                piano(ds5, 1), \
                wait(1),       \
                piano(e5, 1),  \
                wait(1),       \
                piano(b4, 1),  \
                wait(1),       \
                piano(d5, 1),  \
                wait(1),       \
                piano(c5, 1),  \
                wait(1)
#define bar_1_3 piano(a4, 6), \
                piano(a2, 6), \
                wait(1),      \
                piano(e3, 5), \
                wait(1),      \
                piano(a3, 4), \
                wait(1),      \
                piano(c4, 3), \
                wait(1),      \
                piano(e4, 2), \
                wait(1),      \
                piano(a4, 1), \
                wait(1)
#define bar_1_4 piano(b4, 6),  \
                piano(e2, 6),  \
                wait(1),       \
                piano(e3, 5),  \
                wait(1),       \
                piano(gs3, 4), \
                wait(1),       \
                piano(e4, 3),  \
                wait(1),       \
                piano(gs4, 2), \
                wait(1),       \
                piano(b4, 1),  \
                wait(1)
#define bar_1_5 piano(c5, 6),  \
                piano(a2, 6),  \
                wait(1),       \
                piano(e3, 5),  \
                wait(1),       \
                piano(a3, 4),  \
                wait(1),       \
                piano(e4, 3),  \
                wait(1),       \
                piano(e5, 2),  \
                wait(1),       \
                piano(ds5, 1), \
                wait(1)
#define bar_1_6 piano(e5, 1),  \
                wait(1),       \
                piano(ds5, 1), \
                wait(1),       \
                piano(e5, 1),  \
                wait(1),       \
                piano(b4, 1),  \
                wait(1),       \
                piano(d5, 1),  \
                wait(1),       \
                piano(c5, 1),  \
                wait(1)
#define bar_1_7 piano(a4, 6), \
                piano(a2, 6), \
                wait(1),      \
                piano(e3, 5), \
                wait(1),      \
                piano(a3, 4), \
                wait(1),      \
                piano(c4, 3), \
                wait(1),      \
                piano(e4, 2), \
                wait(1),      \
                piano(a4, 1), \
                wait(1)
#define bar_2_1 piano(b4, 6),  \
                piano(e2, 6),  \
                wait(1),       \
                piano(e3, 5),  \
                wait(1),       \
                piano(gs3, 4), \
                wait(1),       \
                piano(d4, 3),  \
                wait(1),       \
                piano(c5, 2),  \
                wait(1),       \
                piano(b4, 1),  \
                wait(1)
#define bar_2_2 piano(a4, 4), \
                piano(a2, 1), \
                wait(1),      \
                piano(e3, 1), \
                wait(1),      \
                piano(a3, 1), \
                wait(2)
#define bar_2_3 piano(a4, 2), \
                piano(a2, 1), \
                wait(1),      \
                piano(e3, 1), \
                wait(1),      \
                piano(a3, 1), \
                wait(1),      \
                piano(b4, 1), \
                wait(1),      \
                piano(c5, 1), \
                wait(1),      \
                piano(d5, 1), \
                wait(1)
#define bar_2_4 piano(e5, 6), \
                piano(c3, 6), \
                wait(1),      \
                piano(g3, 5), \
                wait(1),      \
                piano(c4, 4), \
                wait(1),      \
                piano(g4, 3), \
                wait(1),      \
                piano(f5, 2), \
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

    end()};

void ezpsg_instruments(uint8_t **data)
{
    int8_t instrument = *(*data)++;
    switch (instrument)
    {
    case -1: // piano
        ezpsg_play_note(*(*data)++, *(*data)++,
                        48000u, // duty
                        0x01,   // vol_attack
                        0xF9,   // vol_decay
                        0x31,   // wave_release
                        0);     // pan
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
        ezpsg_tick(11);
        if (!ezpsg_playing())
            break;
    }
}
