/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// gets() is safe with 256 byte buffer
// or use RIA_ATTR_RLN_LENGTH to set a smaller buffer
char s[256];

void main()
{
    clock_t start, time1, time8;
    unsigned int i;
    long rand1, rand2;

    // Test changing PHI2

    start = clock();
    ria_attr_set(1000, RIA_ATTR_PHI2_KHZ);
    for (i = 0; i < 2048; i++)
        ;
    time1 = clock() - start;

    start = clock();
    ria_attr_set(8000, RIA_ATTR_PHI2_KHZ);
    for (i = 0; i < 16384; i++)
        ;
    time8 = clock() - start;

    if (time1 < 14 || time1 > 15 || time8 < 14 || time8 > 15)
        puts("ERROR: RIA_ATTR_PHI2_KHZ failed");
    else
        puts("PASSED: RIA_ATTR_PHI2_KHZ");

    // Test lrand()

    rand1 = ria_attr_get(RIA_ATTR_LRAND);
    for (i = 0; i < 100; i++)
    {
        rand2 = ria_attr_get(RIA_ATTR_LRAND);
        if (rand1 != rand2)
            break;
    }
    if (rand1 == rand2)
        puts("ERROR: RIA_ATTR_LRAND failed");
    else
        puts("PASSED: RIA_ATTR_LRAND");

    // Test gets()

    printf("Enter your name? ");
    gets(s);
    printf("Hello, %s\n", s);

    ria_attr_set(2, RIA_ATTR_RLN_LENGTH);
    printf("Max two characters? ");
    gets(s);
    printf("You typed: %s\n", s);

    ria_attr_set(0, RIA_ATTR_RLN_LENGTH);
    printf("Press enter (you cannot type chars)? ");
    gets(s);
    printf("You pressed enter\n");
}
