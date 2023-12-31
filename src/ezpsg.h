/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EZPSG_H_
#define _EZPSG_H_

#include <stdbool.h>
#include <stdint.h>

enum notes
{
    c2 = 16,
    cs2,
    d2,
    ds2,
    e2,
    f2,
    fs2,
    g2,
    gs2,
    a2,
    as2,
    b2,
    c3,
    cs3,
    d3,
    ds3,
    e3,
    f3,
    fs3,
    g3,
    gs3,
    a3,
    as3,
    b3,
    c4,
    cs4,
    d4,
    ds4,
    e4,
    f4,
    fs4,
    g4,
    gs4,
    a4,
    as4,
    b4,
    c5,
    cs5,
    d5,
    ds5,
    e5,
    f5,
    fs5,
    g5,
    gs5,
    a5,
    as5,
    b5,
};

void ezpsg_init(unsigned addr);
void ezpsg_tick(unsigned tempo);
void ezpsg_play_song(unsigned char *song);
void ezpsg_wait(void);
void ezpsg_play_note(enum notes note,
                     uint16_t duration,
                     uint16_t duty,
                     uint8_t vol_attack,
                     uint8_t vol_decay,
                     uint8_t wave_release,
                     int8_t pan);

void ezpsg_instruments(unsigned char **data);

#endif /* _EZPSG_H_ */
