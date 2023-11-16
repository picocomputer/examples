/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// gets() is safe with 256 byte buffer
// or use stdin_opt() to set a smaller buffer
char s[256];

void main()
{
    printf("Name? ");
    gets(s);
    printf("Hello, %s\n", s);

    stdin_opt(0, 2);
    printf("Max two characters? ");
    gets(s);
    printf("You typed: %s\n", s);

    stdin_opt(0, 0);
    printf("Press enter (you cannot type chars)? ");
    gets(s);
    printf("You pressed enter\n");

    stdin_opt(8, 0);
    while (s[0] != 3)
    {
        printf("Press CTRL-C? ");
        gets(s);
    }
    printf("You pressed CTRL-C\n");
}
