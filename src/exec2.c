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
#include <string.h>

// Up to 512 bytes are needed for argv (one xstack size).
// Applications must opt-in to argc/argv by providing this memory.
void *__fastcall__ argv_mem(size_t size) { return malloc(size); }

int main(int argc, char *argv[])
{
    int i;

    printf("argc = %d\n", argc);
    for (i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]);

    if (argc == 1)
    {
        char arg[] = "Foo";
        printf("Executing self with arg: %s\n", arg);
        ria_execl(argv[0], arg, NULL);
    } else if (argc == 2 && !strcmp("Foo",argv[1])) {
        char arg[] = "Foo2";
        printf("Executing self with arg: %s\n", arg);
        ria_execl(argv[0], arg, NULL);
    } else if (argc == 2 && !strcmp("Foo2",argv[1])) {
        char arg[] = "Foo3";
        printf("Executing self with arg: %s\n", arg);
        ria_execl(argv[0], arg, NULL);
    } else if (argc == 2 && !strcmp("Foo3",argv[1])) {
        char arg[] = "Prefinish";
        printf("Executing self with arg: %s\n", arg);
        ria_execl(argv[0], arg, NULL);
    } else if (argc == 2 && !strcmp("Prefinish",argv[1])) {
        char arg[] = "End";
        printf("Executing self with arg: %s\n", arg);
        ria_execl(argv[0], arg, NULL);
    }
    // The argv memory can be reclaimed.
    // (not useful here in the example)
    free(argv);
    return 1;
}
