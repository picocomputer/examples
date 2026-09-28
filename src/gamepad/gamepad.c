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
    gamepad_player_t pad;
    uint8_t type;

    printf("P%d ", player);

    xram0_read(&pad, XRAM_GAMEPAD + (player - 1) * sizeof(pad), sizeof(pad));

    printf("lx:%4d ly:%4d ", pad.lx, pad.ly);
    printf("rx:%4d ry:%4d ", pad.rx, pad.ry);
    printf("lt:%3u rt:%3u ", pad.l2, pad.r2);
    printf("L:%s ", dpad[pad.sticks & 0xF]);
    printf("R:%s ", dpad[(pad.sticks & 0xF0) >> 4]);
    printf("H:%s ", dpad[pad.dpad & 0xF]);

    type = pad.dpad & GAMEPAD_FEAT_TYPE_MASK;
    printf("%s%s ", types[type >> 4], (pad.dpad & GAMEPAD_FEAT_STICKS) ? "2S" : "  ");

    if (!(pad.dpad & GAMEPAD_FEAT_CONNECTED))
    {
        printf("\33[K\n\033[90m   Disconnected\033[0m\33[K\n\n");
        return;
    }

    printf("\n   ");

    if (type == GAMEPAD_TYPE_PLAYSTATION)
    {
        print(pad.btn0 & GAMEPAD_BTN0_A, "Cross");
        print(pad.btn0 & GAMEPAD_BTN0_B, "Circle");
        print(pad.btn0 & GAMEPAD_BTN0_X, "Square");
        print(pad.btn0 & GAMEPAD_BTN0_Y, "Triangle");
    }
    else
    {
        print(pad.btn0 & GAMEPAD_BTN0_A, "A");
        print(pad.btn0 & GAMEPAD_BTN0_B, "B");
        print(pad.btn0 & GAMEPAD_BTN0_C, "C");
        print(pad.btn0 & GAMEPAD_BTN0_X, "X");
        print(pad.btn0 & GAMEPAD_BTN0_Y, "Y");
        print(pad.btn0 & GAMEPAD_BTN0_Z, "Z");
    }

    print(pad.btn0 & GAMEPAD_BTN0_L1, "L1");
    print(pad.btn0 & GAMEPAD_BTN0_R1, "R1");
    print(pad.btn1 & GAMEPAD_BTN1_L2, "L2");
    print(pad.btn1 & GAMEPAD_BTN1_R2, "R2");

    print(pad.btn1 & GAMEPAD_BTN1_SELECT, "Select");
    print(pad.btn1 & GAMEPAD_BTN1_START, "Start");
    print(pad.btn1 & GAMEPAD_BTN1_HOME, "Home");
    print(pad.btn1 & GAMEPAD_BTN1_L3, "L3");
    print(pad.btn1 & GAMEPAD_BTN1_R3, "R3");

    print(pad.btn1 & 0x80, "?");

    printf("\33[K\n\n");
}

int main(void)
{
    printf("\30\33c\nPicocomputer 6502 Gamepad Tester");
    xreg_ria_gamepad(XRAM_GAMEPAD);
    while (1)
    {
        printf("\33[H\33[3B");
        show(1);
        show(2);
        show(3);
        show(4);
    }
}
