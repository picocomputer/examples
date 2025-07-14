/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <stdint.h>

#define xreg_ria_gamepad(...) xreg(0, 0, 2, __VA_ARGS__)

void show()
{
    const char *dpad[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    uint8_t hat, sticks, btns0, btns1;

    hat = RIA.rw0;
    sticks = RIA.rw0;
    btns0 = RIA.rw0;
    btns1 = RIA.rw0;

    printf("lx:%3u ly:%3u ", RIA.rw0, RIA.rw0);
    printf("rx:%3u ry:%3u ", RIA.rw0, RIA.rw0);
    printf("lt:%3u rt:%3u ", RIA.rw0, RIA.rw0);

    if (!(hat & 0x80))
        printf("Disconnected ");

    if (hat & 0xF < 8)
        printf("D:%s ", dpad[hat & 0xF]);

    if (sticks & 0xF < 8)
        printf("L:%s ", dpad[sticks & 0xF]);

    if ((sticks & 0xF0) >> 4 < 8)
        printf("R:%s ", dpad[(sticks & 0xF0) >> 4]);

    if (hat & 0x40)
    {
        if (btns0 & 0x01)
            printf("Cross ");
        if (btns0 & 0x02)
            printf("Circle ");
        if (btns0 & 0x04)
            printf("Square ");
        if (btns0 & 0x08)
            printf("Triangle ");
    }
    else
    {
        if (btns0 & 0x01)
            printf("A ");
        if (btns0 & 0x02)
            printf("B ");
        if (btns0 & 0x04)
            printf("X ");
        if (btns0 & 0x08)
            printf("Y ");
    }

    if (btns0 & 0x10)
        printf("L1 ");
    if (btns0 & 0x20)
        printf("R1 ");
    if (btns0 & 0x40)
        printf("L2 ");
    if (btns0 & 0x80)
        printf("R2 ");

    if (btns1 & 0x01)
        printf("BK ");
    if (btns1 & 0x02)
        printf("ST ");
    if (btns1 & 0x04)
        printf("L3 ");
    if (btns1 & 0x08)
        printf("R3 ");
    if (btns1 & 0x10)
        printf("() ");

    // Unknown buttons
    if (btns1 & 0xE0)
        printf("0x%02X ", btns1 & 0xE0);

    printf("\n");
}

void main()
{
    unsigned u;
    xreg_ria_gamepad(0xFF00);

    while (1)
    {
        while (--u)
            ;
        RIA.addr0 = 0xFF00;
        RIA.step0 = 1;
        printf("P1 ");
        show();
        printf("P2 ");
        show();
        printf("P3 ");
        show();
        printf("P4 ");
        show();
        printf("\n");
    }
}
