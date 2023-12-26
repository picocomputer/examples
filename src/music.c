/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdint.h>

#define  c2 65
#define cs2 69
#define  d2 73
#define ds2 78
#define  e2 82
#define  f2 87
#define fs2 92
#define  g2 98
#define gs2 104
#define  a2 110
#define as2 117
#define  b2 123

#define  c3 131
#define cs3 139
#define  d3 147
#define ds3 156
#define  e3 165
#define  f3 175
#define fs3 185
#define  g3 196
#define gs3 208
#define  a3 220
#define as3 233
#define  b3 247

#define  c4 262
#define cs4 277
#define  d4 294
#define ds4 311
#define  e4 330
#define  f4 349
#define fs4 370
#define  g4 392
#define gs4 415
#define  a4 440
#define as4 466
#define  b4 494

#define  c5 523
#define cs5 554
#define  d5 587
#define ds5 622
#define  e5 659
#define  f5 698
#define fs5 740
#define  g5 784
#define gs5 831
#define  a5 880
#define as5 932
#define  b5 988


struct channels
{
    uint16_t freq;
    uint16_t duty;
    uint8_t trig_wave;
    uint8_t attack_vol;
    uint8_t decay_vol;
    uint8_t release_pan;
};

void sleep(unsigned n)
{
    unsigned i;
    while (n--)
        for (i = 22000u; i; i--)
            ;
}

play(uint16_t f)
{
    xram0_struct_set(0xFF00, struct channels, freq, f);
    xram0_struct_set(0xFF00, struct channels, trig_wave, 0x12);
}

stop()
{
    xram0_struct_set(0xFF00, struct channels, trig_wave, 0x00);
}

void main(void)
{
    // xreg(0, 1, 0xFF, 0xFF00);

    xram0_struct_set(0xFF00, struct channels, freq, 50);
    xram0_struct_set(0xFF00, struct channels, duty, 65535);
    // xram0_struct_set(0xFF00, struct channels, duty, 32768);
    // xram0_struct_set(0xFF00, struct channels, duty, 5000);
    xram0_struct_set(0xFF00, struct channels, trig_wave, 0x04);
    xram0_struct_set(0xFF00, struct channels, attack_vol, 0x00);
    xram0_struct_set(0xFF00, struct channels, decay_vol, 0x00);
    xram0_struct_set(0xFF00, struct channels, release_pan, 0x00);
    xreg(0, 1, 0x00, 0xFF00);
    sleep(1);

    play(e5);
    sleep(1);
    play(ds5);
    sleep(1);

    play(e5);
    sleep(1);
    play(ds5);
    sleep(1);
    play(e5);
    sleep(1);
    play(b4);
    sleep(1);
    play(d5);
    sleep(1);
    play(c5);
    sleep(1);

    play(a4);
    sleep(2);
    play(a3);
    sleep(1);
    play(c4);
    sleep(1);
    play(e4);
    sleep(1);
    play(a4);
    sleep(1);

    play(b4);
    sleep(2);
    play(gs3);
    sleep(1);
    play(e4);
    sleep(1);
    play(gs4);
    sleep(1);
    play(b4);
    sleep(1);

    play(c5);
    sleep(2);
    play(a3);
    sleep(1);
    play(e4);
    sleep(1);
    play(e5);
    sleep(1);
    play(ds5);
    sleep(1);

    play(e5);
    sleep(1);
    play(ds5);
    sleep(1);
    play(e5);
    sleep(1);
    play(b4);
    sleep(1);
    play(d5);
    sleep(1);
    play(c5);
    sleep(1);

    play(a4);
    sleep(2);

}