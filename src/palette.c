/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <stdio.h>

void main()
{
    int i, j;

    puts("\30\f");
    printf("\33[1mPalette Test\n");
    puts("\33[0m");

    printf("Standard colors       : ");
    for (i = 0; i < 8; i++)
    {
        printf("\33[48:5:%dm      ", i);
    }
    puts("\33[0m");

    printf("High-intensity colors : ");
    for (i = 8; i < 16; i++)
    {
        printf("\33[48:5:%dm      ", i);
    }
    puts("\33[0m");

    printf("\n216 colors (6x6x6) :\n");
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 36; j++)
        {
            printf("\33[48;5;%dm  ", 16 + i * 36 + j);
        }
        puts("\33[0m");
    }

    printf("\nGrayscale colors :\n");
    for (i = 232; i < 256; i++)
    {
        printf("\33[48;5;%dm   ", i);
    }
    puts("\33[0m");

    printf("\n\33[1mRGB Test\n");
    puts("\33[0m");

    printf("256 colors (32x8) :\n");
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 32; j++)
        {
            int r = i * 32;
            int g = j * 8;
            int b = r;
            printf("\33[48;2;%d;%d;%dm  ", r, g, b);
        }
        puts("\33[0m");
    }
    puts("\33[0m");
}
