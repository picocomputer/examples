/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <stdio.h>
#include <time.h>

static void print_time(time_t time)
{
    char buf[100];
    struct tm *tminfo;

    tminfo = gmtime(&time);
    strftime(buf, sizeof(buf), "UTC  : %c", tminfo);
    puts(buf);

    tminfo = localtime(&time);
    strftime(buf, sizeof(buf), "Local: %c %Z", tminfo);
    puts(buf);
}

void main()
{
    puts("This requires RP6502-RIA v0.16 or later.");

    puts("\nThe current time is:");
    print_time(time(NULL));

    puts("\nThis example is standard time:");
    print_time(1735732800); // 1-JAN-2025 noon UTC

    puts("\nDaylight savings time, if observed:");
    print_time(1751371200); // 1-JUL-2025 noon UTC
}
