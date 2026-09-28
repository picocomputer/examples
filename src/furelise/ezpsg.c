/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include "ezpsg.h"
#include "xram.h"
#include <rp6502.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

static struct channel
{
    struct channel *next;
    uint16_t xaddr;
    uint8_t duration;
    uint8_t release;
} ezpsg_channels[PSG_CHANNELS];

static struct channel *ezpsg_channels_free;
static struct channel *ezpsg_channels_playing;
static struct channel *ezpsg_channels_releasing;

static const uint8_t *ezpsg_song;

void ezpsg_init(uint16_t xaddr)
{
    unsigned u;
    // Clear RIA PSG XRAM.
    xram0_set(xaddr, 0, sizeof(psg_t));
    // Start RIA PSG.
    xreg_ria_psg(xaddr);
    // Init linked lists.
    for (u = 0; u < PSG_CHANNELS; u++)
    {
        ezpsg_channels[u].xaddr = xaddr + u * sizeof(psg_channel_t);
        ezpsg_channels[u].next = &ezpsg_channels[u + 1];
    }
    ezpsg_channels[PSG_CHANNELS - 1].next = NULL;
    ezpsg_channels_free = &ezpsg_channels[0];
    ezpsg_channels_playing = NULL;
    ezpsg_channels_releasing = NULL;
    // Clear song.
    ezpsg_song = NULL;
}

bool ezpsg_tick(uint16_t tempo)
{
    static unsigned ticks = 0;
    static unsigned durations = 0;
    struct channel *channel;
    // Just before the last tick we release everything that's done playing.
    if (ticks == 1)
    {
        // A channel is done after its duration countdown.
        while (ezpsg_channels_playing && ezpsg_channels_playing->duration <= 1)
        {
            struct channel **releasing = &ezpsg_channels_releasing;
            // Remove from playing list.
            channel = ezpsg_channels_playing;
            ezpsg_channels_playing = ezpsg_channels_playing->next;
            // Move to releasing list, ordered by release countdown.
            while (*releasing && channel->release > (*releasing)->release)
                releasing = &(*releasing)->next;
            channel->next = *releasing;
            *releasing = channel;
            // Clear gate bit.
            xram0_poke8(channel->xaddr + offsetof(psg_channel_t, pan_gate),
                        xram0_peek8(channel->xaddr + offsetof(psg_channel_t, pan_gate)) &
                            (uint8_t)~PSG_GATE);
        }
        // Decrement everything still playing.
        channel = ezpsg_channels_playing;
        while (channel)
        {
            channel->duration--;
            channel = channel->next;
        }
        ticks--;
        return true;
    }
    // On the final tick of a duration.
    if (ticks == 0)
    {
        // A channel is free after its release countdown.
        while (ezpsg_channels_releasing && ezpsg_channels_releasing->release == 0)
        {
            // Remove from releasing list.
            channel = ezpsg_channels_releasing;
            ezpsg_channels_releasing = ezpsg_channels_releasing->next;
            // Move to free list.
            channel->next = ezpsg_channels_free;
            ezpsg_channels_free = channel;
        }
        // Decrement everything still releasing.
        channel = ezpsg_channels_releasing;
        while (channel)
        {
            channel->release--;
            channel = channel->next;
        }
        // We may have been asked to wait multiple durations.
        if (durations > 1)
            durations--;
        // Play all the instruments then go back to waiting.
        else if (ezpsg_song)
        {
            while ((int8_t)*ezpsg_song < 0)
                ezpsg_instruments(&ezpsg_song);
            if ((int8_t)*ezpsg_song > 0)
                durations = *ezpsg_song++;
        }
        ticks = tempo;
        return true;
    }
    // Tempo+1 ticks per duration/release.
    ticks--;
    return false;
}

uint16_t ezpsg_play_note(uint8_t note,
                         uint8_t duration,
                         uint8_t release,
                         uint8_t duty,
                         uint8_t attack,
                         uint8_t decay,
                         uint8_t release_wave,
                         int8_t pan)
{
    struct channel **playing = &ezpsg_channels_playing;
    psg_channel_t regs;
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
    // Set the countdowns
    channel->duration = duration;
    channel->release = release;
    // Program the XRAM registers
    regs.freq = freq;
    regs.duty = duty;
    regs.attack = attack;
    regs.decay = decay;
    regs.release_wave = release_wave;
    regs.pan_gate = pan | PSG_GATE;
    xram0_write(channel->xaddr, &regs, offsetof(psg_channel_t, reserved));
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
