/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

// Easy Programmable Sound Generator library for RP6502 RIA PSG.
// This is a music tracker and sound effects engine.
// Pros: easy, small, fast, vsync-compatible.
// Cons: advanced users will find it lacking.

#ifndef _EZPSG_H_
#define _EZPSG_H_

#include <stdbool.h>
#include <stdint.h>

enum ezpsg_notes
{
    // TODO add all flat notes
    a0 = 0,
    as0 = 1,
    bb0 = 1,
    b0 = 2,
    c1,
    cs1,
    d1,
    ds1,
    e1,
    f1,
    fs1,
    g1,
    gs1,
    a1,
    as1,
    b1,
    c2,
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
    c6,
    cs6,
    d6,
    ds6,
    e6,
    f6,
    fs6,
    g6,
    gs6,
    a6,
    as6,
    b6,
    c7,
    cs7,
    d7,
    ds7,
    e7,
    f7,
    fs7,
    g7,
    gs7,
    a7,
    as7,
    b7,
    c8,
};

#define EZPSG_NOTE_FREQS 28, 29, 31, 33, 35, 37, 39, 41,                 \
                         44, 46, 49, 52, 55, 58, 62, 65,                 \
                         69, 73, 78, 82, 87, 92, 98, 104,                \
                         110, 117, 123, 131, 139, 147, 156, 165,         \
                         175, 185, 196, 208, 220, 233, 247, 262,         \
                         277, 294, 311, 330, 349, 370, 392, 415,         \
                         440, 466, 494, 523, 554, 587, 622, 659,         \
                         698, 740, 784, 831, 880, 932, 988, 1047,        \
                         1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, \
                         1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, \
                         2794, 2960, 3136, 3322, 3520, 3729, 3951, 4186

// Call init with a valid xram address to turn on RIA PSG sound.
// Call with 0xFFFF to turn off PSG sound. Requires 64 bytes of xram.
void ezpsg_init(uint16_t xaddr);

// Call tick 60-100 times per second. RIA.vsync is usually good enough,
// but a 6522 timer can be used for precision bpm. Tempo is how many ticks+1
// equal one duration unit.
void ezpsg_tick(uint16_t tempo);

// Call play note any time, even from a game engine for sound effects.
// Typically this is used by the instrument callback.
// Pan is -126(left) to +126(right), bit 0 ignored.
void ezpsg_play_note(uint8_t note,
                     uint8_t duration,
                     uint16_t duty,
                     uint8_t vol_attack,
                     uint8_t vol_decay,
                     uint8_t wave_release,
                     int8_t pan);

// Play song will move the song pointer to new music.
void ezpsg_play_song(const uint8_t *song);

// Returns true if a song is playing. Turns false at end of song.
bool ezpsg_playing(void);

// Instruments are implemented by the application using this tracker.
void ezpsg_instruments(const uint8_t **data);

#endif /* _EZPSG_H_ */
