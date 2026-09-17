/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include "xram.h"
#include <rp6502.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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
    const char *types[] = {"?? ", "AB ", "BA ", "PS "};
    uint8_t hat, sticks, btns0, btns1, type;

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

    type = hat & GAMEPAD_FEAT_TYPE_MASK;
    printf("%s%s ", types[type >> 4], (hat & GAMEPAD_FEAT_STICKS) ? "2S" : "  ");

    if (!(hat & GAMEPAD_FEAT_CONNECTED))
    {
        printf("\33[K\n\033[90m   Disconnected\033[0m\33[K\n\n");
        return;
    }

    printf("\n   ");

    if (type == GAMEPAD_TYPE_PLAYSTATION)
    {
        print(btns0 & GAMEPAD_BTN0_A, "Cross");
        print(btns0 & GAMEPAD_BTN0_B, "Circle");
        print(btns0 & GAMEPAD_BTN0_X, "Square");
        print(btns0 & GAMEPAD_BTN0_Y, "Triangle");
    }
    else
    {
        print(btns0 & GAMEPAD_BTN0_A, "A");
        print(btns0 & GAMEPAD_BTN0_B, "B");
        print(btns0 & GAMEPAD_BTN0_C, "C");
        print(btns0 & GAMEPAD_BTN0_X, "X");
        print(btns0 & GAMEPAD_BTN0_Y, "Y");
        print(btns0 & GAMEPAD_BTN0_Z, "Z");
    }

    print(btns0 & GAMEPAD_BTN0_L1, "L1");
    print(btns0 & GAMEPAD_BTN0_R1, "R1");
    print(btns1 & GAMEPAD_BTN1_L2, "L2");
    print(btns1 & GAMEPAD_BTN1_R2, "R2");

    print(btns1 & GAMEPAD_BTN1_SELECT, "Select");
    print(btns1 & GAMEPAD_BTN1_START, "Start");
    print(btns1 & GAMEPAD_BTN1_HOME, "Home");
    print(btns1 & GAMEPAD_BTN1_L3, "L3");
    print(btns1 & GAMEPAD_BTN1_R3, "R3");

    print(btns1 & 0x80, "?");

    printf("\33[K\n\n");
}

int main(void)
{
    printf("\30\33c\nPicocomputer 6502 Gamepad Tester");
    xreg_ria_gamepad(XRAM_GAMEPAD);
    while (1)
    {
        printf("\33[H\33[3B");
        RIA.addr0 = XRAM_GAMEPAD;
        RIA.step0 = 1;
        show(1);
        show(2);
        show(3);
        show(4);
    }
}
