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

struct channel
{
    struct channel *next;
    uint16_t addr;
    uint16_t duration;
    int16_t pan;
} channels[PSG_CHANNELS];

struct channel *channels_free;
struct channel *channels_playing;
struct channel *channels_releasing;

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

static uint16_t notes_enum_to_freq(enum notes note)
{
    switch (note)
    {
    case c2:
        return 65;
    case cs2:
        return 69;
    case d2:
        return 73;
    case ds2:
        return 78;
    case e2:
        return 82;
    case f2:
        return 87;
    case fs2:
        return 92;
    case g2:
        return 98;
    case gs2:
        return 104;
    case a2:
        return 110;
    case as2:
        return 117;
    case b2:
        return 123;
    case c3:
        return 131;
    case cs3:
        return 139;
    case d3:
        return 147;
    case ds3:
        return 156;
    case e3:
        return 165;
    case f3:
        return 175;
    case fs3:
        return 185;
    case g3:
        return 196;
    case gs3:
        return 208;
    case a3:
        return 220;
    case as3:
        return 233;
    case b3:
        return 247;
    case c4:
        return 262;
    case cs4:
        return 277;
    case d4:
        return 294;
    case ds4:
        return 311;
    case e4:
        return 330;
    case f4:
        return 349;
    case fs4:
        return 370;
    case g4:
        return 392;
    case gs4:
        return 415;
    case a4:
        return 440;
    case as4:
        return 466;
    case b4:
        return 494;
    case c5:
        return 523;
    case cs5:
        return 554;
    case d5:
        return 587;
    case ds5:
        return 622;
    case e5:
        return 659;
    case f5:
        return 698;
    case fs5:
        return 740;
    case g5:
        return 784;
    case gs5:
        return 831;
    case a5:
        return 880;
    case as5:
        return 932;
    case b5:
        return 988;
    default:
        return 0;
    }
}

static void debug_list(char *s, struct channel *channels)
{
    printf("%s ", s);
    while (channels)
    {
        printf("%X ", channels->addr);
        channels = channels->next;
    }
    printf("\n");
}

static void wait(void)
{
    unsigned u;
    for (u = 22000u; u; u--)
        ;

    {
        struct channel *note = channels_releasing;
        channels_releasing = NULL;
        while (note)
        {
            struct channel *next = note->next;
            note->next = channels_free;
            channels_free = note;
            note = next;
        }
    }

    while (channels_playing && channels_playing->duration <= 1)
    {
        struct channel *note = channels_playing;
        channels_playing = channels_playing->next;
        note->next = channels_releasing;
        channels_releasing = note;
        xram0_struct_set(note->addr, ria_psg_t, pan_gate, (note->pan & 0xFE));
    }
    {
        struct channel *note = channels_playing;
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
    struct channel *note = channels_free;
    struct channel **insert = &channels_playing;
    if (!note)
    {
#ifdef NDEBUG
        return;
#else
        printf("No free channels\n");
        exit(1);
#endif
    }
    channels_free = note->next;

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

    // debug_list("free", channels_free);
    // debug_list("playing", channels_playing);
    // debug_list("releasing", channels_releasing);
}

static void play(uint8_t note, uint16_t duration)
{
    uint16_t freq = notes_enum_to_freq(note);
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
        channels[u].addr = 0xFF00 + u * sizeof(ria_psg_t);
        channels[u].next = &channels[u + 1];
    }
    channels[PSG_CHANNELS - 1].next = NULL;
    channels_free = &channels[0];
    channels_playing = NULL;
    channels_releasing = NULL;

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
