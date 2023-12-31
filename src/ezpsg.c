/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ezpsg.h"
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

unsigned char *ezpsg_song;

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

void ezpsg_init(unsigned addr)
{
    unsigned u;

    // clear psg xram and start
    RIA.addr0 = addr;
    for (u = 0; u < PSG_CHANNELS * sizeof(ria_psg_t); u++)
        RIA.rw0 = 0;
    xreg(0, 1, 0x00, addr);

    // init linked lists
    for (u = 0; u < PSG_CHANNELS; u++)
    {
        channels[u].addr = addr + u * sizeof(ria_psg_t);
        channels[u].next = &channels[u + 1];
    }
    channels[PSG_CHANNELS - 1].next = NULL;
    channels_free = &channels[0];
    channels_playing = NULL;
    channels_releasing = NULL;

    ezpsg_song = NULL;
}

void ezpsg_play_note(enum notes note,
                     uint16_t duration,
                     uint16_t duty,
                     uint8_t vol_attack,
                     uint8_t vol_decay,
                     uint8_t wave_release,
                     int8_t pan)
{
    uint16_t freq = notes_enum_to_freq(note);
    struct channel *channel = channels_free;
    struct channel **insert = &channels_playing;
    if (!channel)
    {
#ifdef NDEBUG
        return;
#else
        printf("No free channels\n");
        exit(1);
#endif
    }
    channels_free = channel->next;

    channel->duration = duration;
    channel->pan = pan;

    while (*insert && duration > (*insert)->duration)
        insert = &(*insert)->next;
    channel->next = *insert;
    *insert = channel;

    RIA.addr0 = channel->addr;
    RIA.step0 = 1;
    RIA.rw0 = freq & 0xff;
    RIA.rw0 = (freq >> 8) & 0xff;
    RIA.rw0 = duty & 0xff;
    RIA.rw0 = (duty >> 8) & 0xff;
    RIA.rw0 = vol_attack;
    RIA.rw0 = vol_decay;
    RIA.rw0 = wave_release;
    RIA.rw0 = channel->pan | 0x01;
}

void ezpsg_play_song(unsigned char *song)
{
    ezpsg_song = song;
}

void ezpsg_tick(unsigned tempo)
{
    static unsigned ticks = 0;
    static unsigned duration = 0;

    if (!ezpsg_song || !*ezpsg_song)
        return;

    if (ticks == 1)
    {
        struct channel *channel;
        while (channels_playing && channels_playing->duration <= 1)
        {
            channel = channels_playing;
            channels_playing = channels_playing->next;
            channel->next = channels_releasing;
            channels_releasing = channel;
            xram0_struct_set(channel->addr, ria_psg_t, pan_gate, (channel->pan & 0xFE));
        }
        channel = channels_playing;
        while (channel)
        {
            channel->duration--;
            channel = channel->next;
        }
    }

    if (ticks == 0)
    {
        struct channel *note = channels_releasing;
        ticks = tempo;
        channels_releasing = NULL;
        while (note)
        {
            struct channel *next = note->next;
            note->next = channels_free;
            channels_free = note;
            note = next;
        }
        if (duration > 1)
        {
            duration--;
            return;
        }

        while ((int8_t)*ezpsg_song < 0)
            ezpsg_instruments(&ezpsg_song);

        if ((int8_t)*ezpsg_song > 0)
            duration = *ezpsg_song++;

        return;
    }

    ticks--;
}
