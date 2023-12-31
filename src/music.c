/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define c2 65
#define cs2 69
#define d2 73
#define ds2 78
#define e2 82
#define f2 87
#define fs2 92
#define g2 98
#define gs2 104
#define a2 110
#define as2 117
#define b2 123

#define c3 131
#define cs3 139
#define d3 147
#define ds3 156
#define e3 165
#define f3 175
#define fs3 185
#define g3 196
#define gs3 208
#define a3 220
#define as3 233
#define b3 247

#define c4 262
#define cs4 277
#define d4 294
#define ds4 311
#define e4 330
#define f4 349
#define fs4 370
#define g4 392
#define gs4 415
#define a4 440
#define as4 466
#define b4 494

#define c5 523
#define cs5 554
#define d5 587
#define ds5 622
#define e5 659
#define f5 698
#define fs5 740
#define g5 784
#define gs5 831
#define a5 880
#define as5 932
#define b5 988

#define PSG_CHANNELS 8

typedef struct
{
    unsigned int freq;
    unsigned int duty;
    unsigned char vol_attack;
    unsigned char vol_decay;
    unsigned char wave_release;
    unsigned char pan_gate;
} ria_psg_t;

struct note
{
    struct note *next;
    uint16_t addr;
    uint16_t duration;
    int16_t pan;
} notes[PSG_CHANNELS];

struct note *notes_free;
struct note *notes_playing;
struct note *notes_releasing;

static void debug_list(char *s, struct note *notes)
{
    printf("%s ", s);
    while (notes)
    {
        printf("%X ", notes->addr);
        notes = notes->next;
    }
    printf("\n");
}

static void wait(void)
{
    unsigned u;
    for (u = 22000u; u; u--)
        ;

    {
        struct note *note = notes_releasing;
        notes_releasing = NULL;
        while (note)
        {
            struct note *next = note->next;
            note->next = notes_free;
            notes_free = note;
            note = next;
        }
    }

    while (notes_playing && notes_playing->duration <= 1)
    {
        struct note *note = notes_playing;
        notes_playing = notes_playing->next;
        note->next = notes_releasing;
        notes_releasing = note;
        xram0_struct_set(note->addr, ria_psg_t, pan_gate, (note->pan & 0xFE));
    }
    {
        struct note *note = notes_playing;
        while (note)
        {
            note->duration--;
            note = note->next;
        }
    }
}

static void play_note(uint16_t duration,
                      uint16_t freq,
                      uint16_t duty,
                      uint8_t vol_attack,
                      uint8_t vol_decay,
                      uint8_t wave_release,
                      int8_t pan)
{
    struct note *note = notes_free;
    struct note **insert = &notes_playing;
    if (!note)
    {
        printf("No free notes\n");
        exit(1);
    }
    notes_free = note->next;

    note->duration = duration;
    note->pan = pan;

    while (*insert && duration > (*insert)->duration)
        insert = &(*insert)->next;
    note->next = *insert;
    *insert = note;

    RIA.addr0 = note->addr;
    RIA.step0 = 1;
    RIA.rw0 = freq & 0xff;
    RIA.rw0 = (freq >> 8) & 0xff;
    RIA.rw0 = duty & 0xff;
    RIA.rw0 = (duty >> 8) & 0xff;
    RIA.rw0 = vol_attack;
    RIA.rw0 = vol_decay;
    RIA.rw0 = wave_release;
    RIA.rw0 = note->pan | 0x01;

    // debug_list("free", notes_free);
    // debug_list("playing", notes_playing);
    // debug_list("releasing", notes_releasing);
}

static void play(uint16_t freq, uint16_t duration)
{
    play_note(duration, freq,
              48000u, // duty
              0x01,   // vol_attack
              0xF9,   // vol_decay
              0x01,   // wave_release
              0);     // pan
}

void main(void)
{
    unsigned u;

    // clear psg xram and start
    RIA.addr0 = 0xFF00;
    for (u = 0; u < PSG_CHANNELS * sizeof(ria_psg_t); u++)
        RIA.rw0 = 0;
    xreg(0, 1, 0x00, 0xFF00);

    // init linked lists
    for (u = 0; u < PSG_CHANNELS; u++)
    {
        notes[u].addr = 0xFF00 + u * sizeof(ria_psg_t);
        notes[u].next = &notes[u + 1];
    }
    notes[PSG_CHANNELS - 1].next = NULL;
    notes_free = &notes[0];
    notes_playing = NULL;
    notes_releasing = NULL;

    wait();

    // 1-1
    play(e5, 1);
    wait();
    play(ds5, 1);
    wait();
    // 1-2
    play(e5, 1);
    wait();
    play(ds5, 1);
    wait();
    play(e5, 1);
    wait();
    play(b4, 1);
    wait();
    play(d5, 1);
    wait();
    play(c5, 1);
    wait();
    // 1-3
    play(a4, 2);
    play(a2, 6);
    wait();
    play(e3, 5);
    wait();
    play(a3, 4);
    wait();
    play(c4, 3);
    wait();
    play(e4, 2);
    wait();
    play(a4, 1);
    wait();
    // 1-4
    play(b4, 2);
    play(e2, 6);
    wait();
    play(e3, 5);
    wait();
    play(gs3, 4);
    wait();
    play(e4, 3);
    wait();
    play(gs4, 2);
    wait();
    play(b4, 1);
    wait();
    // 1-5
    play(c5, 2);
    play(a2, 6);
    wait();
    play(e3, 5);
    wait();
    play(a3, 4);
    wait();
    play(e4, 3);
    wait();
    play(e5, 2);
    wait();
    play(ds5, 1);
    wait();
    // 1-6
    play(e5, 1);
    wait();
    play(ds5, 1);
    wait();
    play(e5, 1);
    wait();
    play(b4, 1);
    wait();
    play(d5, 1);
    wait();
    play(c5, 1);
    wait();
    // 1-7
    play(a4, 2);
    play(a2, 6);
    wait();
    play(e3, 5);
    wait();
    play(a3, 4);
    wait();
    play(c4, 3);
    wait();
    play(e4, 2);
    wait();
    play(a4, 1);
    wait();

    // 2-1
    play(b4, 2);
    play(e2, 6);
    wait();
    play(e3, 5);
    wait();
    play(gs3, 4);
    wait();
    play(d4, 3);
    wait();
    play(c5, 2);
    wait();
    play(b4, 1);
    wait();

    // 2-2
    play(a4, 4);
    play(a2, 2);
    wait();
    play(e3, 2);
    wait();
    play(a3, 2);
    wait();
    wait();

    // 1-1
    play(e5, 1);
    wait();
    play(ds5, 1);
    wait();
    // 1-2
    play(e5, 1);
    wait();
    play(ds5, 1);
    wait();
    play(e5, 1);
    wait();
    play(b4, 1);
    wait();
    play(d5, 1);
    wait();
    play(c5, 1);
    wait();
    // 1-3
    play(a4, 2);
    play(a2, 6);
    wait();
    play(e3, 5);
    wait();
    play(a3, 4);
    wait();
    play(c4, 3);
    wait();
    play(e4, 2);
    wait();
    play(a4, 1);
    wait();
    // 1-4
    play(b4, 2);
    play(e2, 6);
    wait();
    play(e3, 5);
    wait();
    play(gs3, 4);
    wait();
    play(e4, 3);
    wait();
    play(gs4, 2);
    wait();
    play(b4, 1);
    wait();
    // 1-5
    play(c5, 2);
    play(a2, 6);
    wait();
    play(e3, 5);
    wait();
    play(a3, 4);
    wait();
    play(e4, 3);
    wait();
    play(e5, 2);
    wait();
    play(ds5, 1);
    wait();
    // 1-6
    play(e5, 1);
    wait();
    play(ds5, 1);
    wait();
    play(e5, 1);
    wait();
    play(b4, 1);
    wait();
    play(d5, 1);
    wait();
    play(c5, 1);
    wait();
    // 1-7
    play(a4, 2);
    play(a2, 6);
    wait();
    play(e3, 5);
    wait();
    play(a3, 4);
    wait();
    play(c4, 3);
    wait();
    play(e4, 2);
    wait();
    play(a4, 1);
    wait();

    // 2-1
    play(b4, 2);
    play(e2, 6);
    wait();
    play(e3, 5);
    wait();
    play(gs3, 4);
    wait();
    play(d4, 3);
    wait();
    play(c5, 2);
    wait();
    play(b4, 1);
    wait();

    // 2-3
    play(a4, 2);
    play(a2, 1);
    wait();
    play(e3, 1);
    wait();
    play(a3, 1);
    wait();
    play(b4, 1);
    wait();
    play(c5, 1);
    wait();
    play(d5, 1);
    wait();

    // 2-4
    play(e5, 3);
    play(c3, 6);
    wait();
    play(g3, 5);
    wait();
    play(c4, 4);
    wait();
    play(g4, 3);
    wait();
    play(f5, 2);
    wait();
    play(e5, 1);
    wait();
}
