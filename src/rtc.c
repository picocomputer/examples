/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

 #include <rp6502.h>
#include <stdio.h>
#include <time.h>

static void print_time(time_t time)
{
    char buf[100];
    struct tm *tminfo;

    // UTC requires no effort
    tminfo = gmtime(&time);
    strftime(buf, sizeof(buf), "%c", tminfo);
    printf("UTC  : %s\n", buf);

    // CC65 doesn't have a time library that fully supports
    // daylight savings time. This small hack uses the RP6502
    // operating system to compute DST for a time_t. It sets
    // the global _tz and therefore must be updated for each
    // different time before DST aware operations.
    ria_tzset(time); // hack part 1/2
    tminfo = localtime(&time);
    // One would assume localtime copies this data. It does not.
    tminfo->tm_isdst = _tz.daylight; // hack part 2/2
    strftime(buf, sizeof(buf), "%c %Z", tminfo);
    printf("Local: %s\n", buf);
}

void main()
{
    puts("The current time is:");
    print_time(time(NULL));

    puts("\nThis example is standard time:");
    print_time(1735732800); // 1-JAN-2025 noon UTC

    puts("\nDaylight savings time, if observed:");
    print_time(1751371200); // 1-JUL-2025 noon UTC
}
