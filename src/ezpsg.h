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
    a0 = 0,
    as0 = 1,
    bb0 = 1,
    b0 = 2,
    c1 = 3,
    cs1 = 4,
    db1 = 4,
    d1 = 5,
    ds1 = 6,
    eb1 = 6,
    e1 = 7,
    f1 = 8,
    fs1 = 9,
    gb1 = 9,
    g1 = 10,
    gs1 = 11,
    ab1 = 11,
    a1 = 12,
    as1 = 13,
    bb1 = 13,
    b1 = 14,
    c2 = 15,
    cs2 = 16,
    db2 = 16,
    d2 = 17,
    ds2 = 18,
    eb2 = 18,
    e2 = 19,
    f2 = 20,
    fs2 = 21,
    gb2 = 21,
    g2 = 22,
    gs2 = 23,
    ab2 = 23,
    a2 = 24,
    as2 = 25,
    bb2 = 25,
    b2 = 26,
    c3 = 27,
    cs3 = 28,
    db3 = 28,
    d3 = 29,
    ds3 = 30,
    eb3 = 30,
    e3 = 31,
    f3 = 32,
    fs3 = 33,
    gb3 = 33,
    g3 = 34,
    gs3 = 35,
    ab3 = 35,
    a3 = 36,
    as3 = 37,
    bb3 = 37,
    b3 = 38,
    c4 = 39,
    cs4 = 40,
    db4 = 40,
    d4 = 41,
    ds4 = 42,
    eb4 = 42,
    e4 = 43,
    f4 = 44,
    fs4 = 45,
    gb4 = 45,
    g4 = 46,
    gs4 = 47,
    ab4 = 47,
    a4 = 48,
    as4 = 49,
    bb4 = 49,
    b4 = 50,
    c5 = 51,
    cs5 = 52,
    db5 = 52,
    d5 = 53,
    ds5 = 54,
    eb5 = 54,
    e5 = 55,
    f5 = 56,
    fs5 = 57,
    gb5 = 57,
    g5 = 58,
    gs5 = 59,
    ab5 = 59,
    a5 = 60,
    as5 = 61,
    bb5 = 61,
    b5 = 62,
    c6 = 63,
    cs6 = 64,
    db6 = 64,
    d6 = 65,
    ds6 = 66,
    eb6 = 66,
    e6 = 67,
    f6 = 68,
    fs6 = 69,
    gb6 = 69,
    g6 = 70,
    gs6 = 71,
    ab6 = 71,
    a6 = 72,
    as6 = 73,
    bb6 = 73,
    b6 = 74,
    c7 = 75,
    cs7 = 76,
    db7 = 76,
    d7 = 77,
    ds7 = 78,
    eb7 = 78,
    e7 = 79,
    f7 = 80,
    fs7 = 81,
    gb7 = 81,
    g7 = 82,
    gs7 = 83,
    ab7 = 83,
    a7 = 84,
    as7 = 85,
    bb7 = 85,
    b7 = 86,
    c8 = 87,
};

#define EZPSG_NOTE_FREQS 83, 87, 93, 98, 104, 110, 117, 124,             \
                         131, 139, 147, 156, 165, 175, 185, 196,         \
                         208, 220, 233, 247, 262, 278, 294, 311,         \
                         330, 350, 370, 392, 416, 440, 467, 494,         \
                         524, 555, 588, 623, 660, 699, 741, 785,         \
                         832, 881, 933, 989, 1048, 1110, 1176, 1246,     \
                         1320, 1398, 1482, 1570, 1663, 1762, 1867, 1978, \
                         2095, 2220, 2352, 2492, 2640, 2797, 2963, 3140, \
                         3326, 3524, 3734, 3956, 4191, 4440, 4704, 4984, \
                         5280, 5594, 5927, 6279, 6652, 7048, 7467, 7911, \
                         8381, 8880, 9408, 9967, 10560, 11188, 11853, 12558

// Call init with a valid xram address to turn on RIA PSG sound.
// Requires 64 bytes of xram.
void ezpsg_init(uint16_t xaddr);

// Call tick 60-100 times per second. RIA.vsync is usually good enough,
// but a 6522 timer can be used for precision bpm. Tempo is how many ticks+1
// equal one duration unit. Return value is true when work was done. Work is
// always done in two adjacent ticks once every duration unit. Use the return
// value to defer other non-time-critical tasks in a game loop.
bool ezpsg_tick(uint16_t tempo);

// Call play note at any time, even from a game engine for sound effects.
// Returns XRAM address of PSG config structure used for playback.
// Returns 0xFFFF when no oscillator channels are available.
// Pan is -126(left) to +126(right), bit 0 ignored.
uint16_t ezpsg_play_note(uint8_t note,
                         uint8_t duration,
                         uint8_t release,
                         uint8_t duty,
                         uint8_t vol_attack,
                         uint8_t vol_decay,
                         uint8_t wave_release,
                         int8_t pan);

// Play song will move the song pointer to new music.
void ezpsg_play_song(const uint8_t *song);

// Returns true if a song is playing. Turns false at end of song.
bool ezpsg_playing(void);

// Instruments are implemented in this callback.
// See furelise.c and poprock.c examples.
void ezpsg_instruments(const uint8_t **data);

#endif /* _EZPSG_H_ */
