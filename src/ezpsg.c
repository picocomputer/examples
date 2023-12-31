/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
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

static struct channel
{
    struct channel *next;
    uint16_t xaddr;
    uint8_t duration;
} channels[PSG_CHANNELS];

static struct channel *channels_free;
static struct channel *channels_playing;
static struct channel *channels_releasing;

static const uint8_t *ezpsg_song;

static uint16_t note_to_freq(uint8_t note)
{
    static const uint16_t conv[] = {EZPSG_NOTE_FREQS};
    return conv[note];
}

void ezpsg_init(uint16_t xaddr)
{
    unsigned u;

    // clear psg xram and start
    RIA.addr0 = xaddr;
    for (u = 0; u < PSG_CHANNELS * sizeof(ria_psg_t); u++)
        RIA.rw0 = 0;
    xreg(0, 1, 0x00, xaddr);

    // init linked lists
    for (u = 0; u < PSG_CHANNELS; u++)
    {
        channels[u].xaddr = xaddr + u * sizeof(ria_psg_t);
        channels[u].next = &channels[u + 1];
    }
    channels[PSG_CHANNELS - 1].next = NULL;
    channels_free = &channels[0];
    channels_playing = NULL;
    channels_releasing = NULL;

    // clear song
    ezpsg_song = NULL;
}

void ezpsg_tick(uint16_t tempo)
{
    static unsigned ticks = 0;
    static unsigned waits = 0;

    if (ticks == 1)
    {
        struct channel *channel;
        while (channels_playing && channels_playing->duration <= 1)
        {
            channel = channels_playing;
            channels_playing = channels_playing->next;
            channel->next = channels_releasing;
            channels_releasing = channel;
            RIA.addr0 = channel->xaddr + (unsigned)(&((ria_psg_t *)0)->pan_gate);
            RIA.step0 = 0;
            RIA.rw0 &= 0xFE;
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
        if (waits > 1)
        {
            waits--;
            return;
        }
        if (ezpsg_song)
        {
            while ((int8_t)*ezpsg_song < 0)
                ezpsg_instruments(&ezpsg_song);

            if ((int8_t)*ezpsg_song > 0)
                waits = *ezpsg_song++;
        }
        return;
    }

    ticks--;
}

void ezpsg_play_note(uint8_t note,
                     uint8_t duration,
                     uint16_t duty,
                     uint8_t vol_attack,
                     uint8_t vol_decay,
                     uint8_t wave_release,
                     int8_t pan)
{
    uint16_t freq = note_to_freq(note);
    struct channel *channel = channels_free;
    struct channel **insert = &channels_playing;
    if (!channel)
    {
#ifdef NDEBUG
        return;
#else
        puts("No free channels.");
        exit(1);
#endif
    }
    channels_free = channel->next;

    while (*insert && duration > (*insert)->duration)
        insert = &(*insert)->next;
    channel->next = *insert;
    *insert = channel;

    channel->duration = duration;

    RIA.addr0 = channel->xaddr;
    RIA.step0 = 1;
    RIA.rw0 = freq & 0xff;
    RIA.rw0 = (freq >> 8) & 0xff;
    RIA.rw0 = duty & 0xff;
    RIA.rw0 = (duty >> 8) & 0xff;
    RIA.rw0 = vol_attack;
    RIA.rw0 = vol_decay;
    RIA.rw0 = wave_release;
    RIA.rw0 = pan | 0x01;
}

void ezpsg_play_song(const uint8_t *song)
{
    ezpsg_song = song;
}

bool ezpsg_playing(void)
{
    return (ezpsg_song && *ezpsg_song) || channels_playing;
}
