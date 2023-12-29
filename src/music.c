/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
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

typedef struct
{
    uint16_t duty;
    uint16_t freq;
    uint8_t pan_gate;
    uint8_t vol_attack;
    uint8_t vol_decay;
    uint8_t wave_release;
} ria_psg_t;

void sleep(unsigned n)
{
    unsigned i;
    while (n--)
        for (i = 22000u; i; i--)
            ;
}

void play(uint16_t f)
{
    xram0_struct_set(0xFF00, ria_psg_t, freq, f);
    xram0_struct_set(0xFF00, ria_psg_t, pan_gate, 0x01);
}

void stop()
{
    unsigned i;
    xram0_struct_set(0xFF00, ria_psg_t, pan_gate, 0);
    for (i = 1000u; i; i--)
        ;
}

void main(void)
{
    unsigned u;
    // xreg(0, 1, 0xFF, 0xFF00);

    for (u = 0; u < 8; u++)
    {
        unsigned base = 0xFF00 + sizeof(ria_psg_t) * u;
        // printf("%X\n", base);
        xram0_struct_set(base, ria_psg_t, vol_attack, 0x01);
        xram0_struct_set(base, ria_psg_t, vol_decay, 0xF9);
        xram0_struct_set(base, ria_psg_t, wave_release, 0x30);
        xram0_struct_set(base, ria_psg_t, pan_gate, 0x00);
        xram0_struct_set(base, ria_psg_t, freq, 50);
        // xram0_struct_set(base, ria_psg_t, duty, 65535u);
        xram0_struct_set(base, ria_psg_t, duty, 48000u);
        // xram0_struct_set(base, ria_psg_t, duty, 32768u);
        // xram0_struct_set(base, ria_psg_t, duty, 15000u);
    }

    xreg(0, 1, 0x00, 0xFF00);
    sleep(1);

    stop();
    play(e5);
    sleep(1);
    stop();
    play(ds5);
    sleep(1);

    stop();
    play(e5);
    sleep(1);
    stop();
    play(ds5);
    sleep(1);
    stop();
    play(e5);
    sleep(1);
    stop();
    play(b4);
    sleep(1);
    stop();
    play(d5);
    sleep(1);
    stop();
    play(c5);
    sleep(1);

    stop();
    play(a4);
    sleep(2);
    stop();
    play(a3);
    sleep(1);
    stop();
    play(c4);
    sleep(1);
    stop();
    play(e4);
    sleep(1);
    stop();
    play(a4);
    sleep(1);

    stop();
    play(b4);
    sleep(2);
    stop();
    play(gs3);
    sleep(1);
    stop();
    play(e4);
    sleep(1);
    stop();
    play(gs4);
    sleep(1);
    stop();
    play(b4);
    sleep(1);

    stop();
    play(c5);
    sleep(2);
    stop();
    play(a3);
    sleep(1);
    stop();
    play(e4);
    sleep(1);
    stop();
    play(e5);
    sleep(1);
    stop();
    play(ds5);
    sleep(1);

    stop();
    play(e5);
    sleep(1);
    stop();
    play(ds5);
    sleep(1);
    stop();
    play(e5);
    sleep(1);
    stop();
    play(b4);
    sleep(1);
    stop();
    play(d5);
    sleep(1);
    stop();
    play(c5);
    sleep(1);

    stop();
    play(a4);
    sleep(2);
}
