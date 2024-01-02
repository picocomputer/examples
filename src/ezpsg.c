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
} ezpsg_channels[PSG_CHANNELS];

static struct channel *ezpsg_channels_free;
static struct channel *ezpsg_channels_playing;
static struct channel *ezpsg_channels_releasing;

static const uint8_t *ezpsg_song;

void ezpsg_init(uint16_t xaddr)
{
    unsigned u;
    // Clear RIA PSG XRAM.
    RIA.addr0 = xaddr;
    RIA.step0 = 1;
    for (u = 0; u < PSG_CHANNELS * sizeof(ria_psg_t); u++)
        RIA.rw0 = 0;
    // Start RIA PSG.
    xreg(0, 1, 0x00, xaddr);
    // Init linked lists.
    for (u = 0; u < PSG_CHANNELS; u++)
    {
        ezpsg_channels[u].xaddr = xaddr + u * sizeof(ria_psg_t);
        ezpsg_channels[u].next = &ezpsg_channels[u + 1];
    }
    ezpsg_channels[PSG_CHANNELS - 1].next = NULL;
    ezpsg_channels_free = &ezpsg_channels[0];
    ezpsg_channels_playing = NULL;
    ezpsg_channels_releasing = NULL;
    // Clear song.
    ezpsg_song = NULL;
}

void ezpsg_tick(uint16_t tempo)
{
    static unsigned ticks = 0;
    static unsigned durations = 0;
    // Just before the last tick we release everything that's done playing.
    if (ticks == 1)
    {
        // A channel is done after its duration countdown.
        struct channel *channel;
        while (ezpsg_channels_playing && ezpsg_channels_playing->duration <= 1)
        {
            // Remove from playing list.
            channel = ezpsg_channels_playing;
            ezpsg_channels_playing = ezpsg_channels_playing->next;
            // Move to releasing list.
            channel->next = ezpsg_channels_releasing;
            ezpsg_channels_releasing = channel;
            // Clear gate bit.
            RIA.addr0 = channel->xaddr + (unsigned)(&((ria_psg_t *)0)->pan_gate);
            RIA.step0 = 0;
            RIA.rw0 &= 0xFE;
        }
        // Decrement everything still playing.
        channel = ezpsg_channels_playing;
        while (channel)
        {
            channel->duration--;
            channel = channel->next;
        }
    }
    // On the final tick of a duration.
    if (ticks == 0)
    {
        // Move all the releasing channels back to the free list.
        // They have had 1 tick, about 10-17ms, to release. This greatly
        // simplifies the logic but limits the release rate to 0 (6ms).
        struct channel *releasing = ezpsg_channels_releasing;
        ticks = tempo;
        ezpsg_channels_releasing = NULL;
        while (releasing)
        {
            struct channel *next = releasing->next;
            releasing->next = ezpsg_channels_free;
            ezpsg_channels_free = releasing;
            releasing = next;
        }
        // We may have been asked to wait multiple durations.
        if (durations > 1)
        {
            durations--;
            return;
        }
        // Play all the instruments then go back to waiting.
        if (ezpsg_song)
        {
            while ((int8_t)*ezpsg_song < 0)
                ezpsg_instruments(&ezpsg_song);
            if ((int8_t)*ezpsg_song > 0)
                durations = *ezpsg_song++;
        }
        return;
    }
    // Tempo+1 ticks per duration.
    ticks--;
}

uint16_t ezpsg_play_note(uint8_t note,
                         uint8_t duration,
                         uint16_t duty,
                         uint8_t vol_attack,
                         uint8_t vol_decay,
                         uint8_t wave_release,
                         int8_t pan)
{
    struct channel **playing = &ezpsg_channels_playing;
    // Convert note to frequency in hertz
    static const uint16_t freq_conv[] = {EZPSG_NOTE_FREQS};
    uint16_t freq = freq_conv[note];
    // Obtain a free channel or do nothing
    struct channel *channel = ezpsg_channels_free;
    if (!channel)
        return 0xFFFF;
    ezpsg_channels_free = channel->next;
    // Move channel into playling list, ordered by duration
    while (*playing && duration > (*playing)->duration)
        playing = &(*playing)->next;
    channel->next = *playing;
    *playing = channel;
    // Set the duration countdown
    channel->duration = duration;
    // Program the XRAM registers
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
    // Success. The caller may manipulate the returned
    // channel until the tick which clears the gate.
    return channel->xaddr;
}

void ezpsg_play_song(const uint8_t *song)
{
    ezpsg_song = song;
}

bool ezpsg_playing(void)
{
    return (ezpsg_song && *ezpsg_song) || ezpsg_channels_playing;
}
