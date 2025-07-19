/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define xreg_ria_gamepad(...) xreg(0, 0, 2, __VA_ARGS__)

void print(bool enabled, const char *str)
{
    if (enabled)
    {
        // Green background with black text for better visibility
        printf("\033[30;42m%s\033[0m ", str);
    }
    else
    {
        // Print bright black text (dark gray) instead of dim
        printf("\033[90m%s\033[0m ", str);
    }
}

void show(int player)
{
    const char *dpad[] = {"0 ", "N ", "S ", "ER",
                          "W ", "NW", "SW", "ER",
                          "E ", "NE", "SE", "ER",
                          "ER", "ER", "ER", "ER"};
    uint8_t hat, sticks, btns0, btns1;

    printf("P%d ", player);

    hat = RIA.rw0;
    sticks = RIA.rw0;
    btns0 = RIA.rw0;
    btns1 = RIA.rw0;

    printf("lx:%4d ly:%4d ", (int8_t)RIA.rw0, (int8_t)RIA.rw0);
    printf("rx:%4d ry:%4d ", (int8_t)RIA.rw0, (int8_t)RIA.rw0);
    printf("lt:%3u rt:%3u ", RIA.rw0, RIA.rw0);
    printf("L:%s ", dpad[sticks & 0xF]);
    printf("R:%s ", dpad[(sticks & 0xF0) >> 4]);
    printf("H:%s ", dpad[hat & 0xF]);

    if (!(hat & 0x80))
    {
        printf("\33[K\n\033[90m   Disconnected\033[0m\33[K\n\n");
        return;
    }

    printf("\n   ");

    if (hat & 0x40)
    {
        print(btns0 & 0x01, "Cross");
        print(btns0 & 0x02, "Circle");
        print(btns0 & 0x04, "Square");
        print(btns0 & 0x08, "Triangle");
    }
    else
    {
        print(btns0 & 0x01, "A");
        print(btns0 & 0x02, "B");
        print(btns0 & 0x04, "X");
        print(btns0 & 0x08, "Y");
    }

    print(btns0 & 0x10, "L1");
    print(btns0 & 0x20, "R1");
    print(btns0 & 0x40, "Select");
    print(btns0 & 0x80, "Start");

    print(btns1 & 0x01, "L2");
    print(btns1 & 0x02, "R2");
    print(btns1 & 0x04, "L3");
    print(btns1 & 0x08, "R3");
    print(btns1 & 0x10, "Home");

    // Unknown buttons
    if (btns1 & 0xE0)
        printf("\033[90m0x%02X\033[0m", btns1 & 0xE0);
    printf("\33[K\n\n");
}

void main()
{
    printf("\30\33cPicocomputer 6502 Gamepad Tester");
    xreg_ria_gamepad(0xFF00);
    while (1)
    {
        printf("\33[H\33[2B");
        RIA.addr0 = 0xFF00;
        RIA.step0 = 1;
        show(1);
        show(2);
        show(3);
        show(4);
    }
}
