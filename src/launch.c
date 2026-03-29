/*
 * Copyright (c) 2026 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

// Up to 512 bytes are needed for argv (one xstack size).
// Applications must opt-in to argc/argv by providing this memory.
void *__fastcall__ argv_mem(size_t size) { return malloc(size); }

int main(int argc, char *argv[])
{
    int i;

    printf("argc = %d\n", argc);
    for (i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);

    if (argc == 2)
    {
        printf("Success\n");
        return 0;
    }

    if (argc == 1)
    {
        ria_attr_set(1, RIA_ATTR_LAUNCHER);
        printf("Executing palette.rp6502\n");
        ria_execl("palette.rp6502", NULL);
    }

    // The argv memory can be reclaimed.
    // (not useful here in the example)
    free(argv);
    return 1;
}
