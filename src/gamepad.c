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
    const char *dpad[] = {"N ", "NE ", "E ", "SE ", "S ", "SW ", "W ", "NW ", "",
                          "Error", "Error", "Error", "Error", "Error", "Error",
                          "Disconnected"};
    uint8_t hat, btns0, btns1, btns2;

    printf("lx:%3u ly:%3u ", RIA.rw0, RIA.rw0);
    printf("rx:%3u ry:%3u ", RIA.rw0, RIA.rw0);
    printf("lt:%3u rt:%3u ", RIA.rw0, RIA.rw0);
    hat = RIA.rw0;
    btns0 = RIA.rw0;
    btns1 = RIA.rw0;
    btns2 = RIA.rw0;

    printf("%s", dpad[hat & 0xf]);

    if (btns0 & 0x01)
        printf("A ");
    if (btns0 & 0x02)
        printf("B ");
    if (btns0 & 0x04)
        printf("X ");
    if (btns0 & 0x08)
        printf("Y ");
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

    if (btns1 & 0xE0)
        printf("1:%02X ", btns1 & 0xE0);

    if (btns2)
        printf("2:%02X ", btns2);

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
