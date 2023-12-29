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
    uint16_t duty;
    uint16_t freq;
    uint8_t pan_gate;
    uint8_t vol_attack;
    uint8_t vol_decay;
    uint8_t wave_release;
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

static void
play_piano(uint16_t f, int8_t pan)
{
    xram0_struct_set(0xFF00, ria_psg_t, duty, 48000u);
    xram0_struct_set(0xFF00, ria_psg_t, freq, f);
    xram0_struct_set(0xFF00, ria_psg_t, vol_attack, 0x01);
    xram0_struct_set(0xFF00, ria_psg_t, vol_decay, 0xF9);
    xram0_struct_set(0xFF00, ria_psg_t, wave_release, 0x31);
    xram0_struct_set(0xFF00, ria_psg_t, pan_gate, (uint8_t)(pan | 0x01));
}

void wait(unsigned n)
{
    unsigned u;
    while (n--)
        for (u = 22000u; u; u--)
            ;
    xram0_struct_set(0xFF00, ria_psg_t, pan_gate, 0);
    for (u = 1000u; u; u--)
        ;
}

void debug_list(char *s, struct note *notes)
{
    printf("%s ", s);
    while (notes)
    {
        printf("%X ", notes->addr);
        notes = notes->next;
    }
    printf("\n");
}

void play(uint16_t freq, uint16_t duration)
{
    struct note *note = notes_free;
    struct note **insert = &notes_playing;
    if (!note)
    {
        printf("No free notes");
        exit(1);
    }
    notes_free = note->next;

    while (*insert && duration > (*insert)->duration)
        insert = &(*insert)->next;
    if (*insert && (*insert)->next)
    {
        note->next = (*insert)->next;
        *insert = note;
    }
    else
    {
        note->next = NULL;
        *insert = note;
    }

    debug_list("free", notes_free);
    debug_list("playing", notes_playing);

    play_piano(freq, 0);
}

void main(void)
{
    unsigned u;

    // clear psg xram and start
    RIA.addr0 = 0xFF00;
    for (u = 0; u < PSG_CHANNELS * sizeof(ria_psg_t); u++)
        RIA.rw0 = 0;
    xreg(0, 1, 0x00, 0xFF00);

    for (u = 0; u < PSG_CHANNELS; u++)
    {
        notes[u].addr = 0xFF00 + u * sizeof(ria_psg_t);
        notes[u].next = &notes[u + 1];
    }
    notes[PSG_CHANNELS - 1].next = NULL;
    notes_free = &notes[0];
    notes_playing = NULL;
    notes_releasing = NULL;

    wait(1);

    play(e5, 1);
    wait(1);
    play(ds5, 1);
    wait(1);

    play(e5, 1);
    wait(1);
    play(ds5, 1);
    wait(1);
    play(e5, 1);
    wait(1);
    play(b4, 1);
    wait(1);
    play(d5, 1);
    wait(1);
    play(c5, 1);
    wait(1);

    play(a4, 1);
    wait(2);
    play(a3, 1);
    wait(1);
    play(c4, 1);
    wait(1);
    play(e4, 1);
    wait(1);
    play(a4, 1);
    wait(1);

    play(b4, 1);
    wait(2);
    play(gs3, 1);
    wait(1);
    play(e4, 1);
    wait(1);
    play(gs4, 1);
    wait(1);
    play(b4, 1);
    wait(1);

    play(c5, 1);
    wait(2);
    play(a3, 1);
    wait(1);
    play(e4, 1);
    wait(1);
    play(e5, 1);
    wait(1);
    play(ds5, 1);
    wait(1);

    play(e5, 1);
    wait(1);
    play(ds5, 1);
    wait(1);
    play(e5, 1);
    wait(1);
    play(b4, 1);
    wait(1);
    play(d5, 1);
    wait(1);
    play(c5, 1);
    wait(1);

    play(a4, 1);
    wait(2);
}
