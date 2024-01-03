/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include "ezpsg.h"
#include <rp6502.h>
#include <stdlib.h>
#include <stdio.h>

#define wait(duration) (duration)
#define hihat() (-1)
#define kick(duration) (-2), (duration)
#define snare() (-3)
#define end() (0)

#define bar_1 hihat(), \
              wait(4), \
              hihat(), \
              wait(4)

#define bar_2 hihat(),  \
              kick(12), \
              wait(4),  \
              hihat(),  \
              wait(4),  \
              hihat(),  \
              snare(),  \
              wait(4),  \
              hihat(),  \
              wait(4)

#define bar_3 hihat(), \
              wait(4), \
              hihat(), \
              kick(7), \
              wait(4), \
              hihat(), \
              snare(), \
              wait(4), \
              hihat(), \
              kick(3), \
              wait(4)

#define bar_4 hihat(),  \
              kick(1),  \
              wait(2),  \
              kick(12), \
              wait(2),  \
              hihat(),  \
              wait(4),  \
              hihat(),  \
              snare(),  \
              wait(4),  \
              hihat(),  \
              wait(4)

static const uint8_t song[] = {
    wait(1),

    bar_1,
    bar_2,
    bar_3,
    bar_4,
    bar_3,
    bar_4,
    bar_2,
    bar_3,
    bar_4,

    end()};

void ezpsg_instruments(const uint8_t **data)
{
    switch ((int8_t) * (*data)++) // instrument
    {
    case -1:                  // hihat
        ezpsg_play_note(e5,   // note
                        2,    // duration
                        0,    // release
                        12,   // duty
                        0x61, // vol_attack
                        0xF7, // vol_decay
                        0x10, // wave_release
                        0);   // pan
        break;
    case -2:                        // kick
        ezpsg_play_note(d1,         // note
                        *(*data)++, // duration
                        0,          // release
                        32,         // duty
                        0x01,       // vol_attack
                        0xF9,       // vol_decay
                        0x40,       // wave_release
                        0);         // pan
        break;
    case -3:                  // snare
        ezpsg_play_note(c3,   // note
                        4,    // duration
                        0,    // release
                        64,   // duty
                        0x01, // vol_attack
                        0xF8, // vol_decay
                        0x40, // wave_release
                        0);   // pan
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
        ezpsg_tick(3);
        if (!ezpsg_playing())
            break;
    }
}
