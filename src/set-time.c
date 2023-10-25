#include <rp6502.h>
#include <clock.h>
#include <stdio.h>
#include <stdlib.h>

#define IN_BUF_SIZE 8

void main()
{
    struct tm current_time;
    struct timespec now;
    char in_buf[IN_BUF_SIZE];

    current_time.tm_hour = -1;
    current_time.tm_sec = -1;
    current_time.tm_min = -1;
    current_time.tm_mday = 0;
    current_time.tm_mon = -1;
    current_time.tm_year = -1;
    current_time.tm_isdst = -1;
    current_time.tm_wday = 0;
    current_time.tm_yday = 0;


    while (current_time.tm_mon < 0) {
        puts("Enter month (1-12): ");
        gets(in_buf);
        current_time.tm_mon = atoi(in_buf) - 1;
        if (current_time.tm_mon > 11)
            current_time.tm_mon = -1;
    }
    while (!current_time.tm_mday) {
        puts("Enter day (1-31): ");
        gets(in_buf);
        current_time.tm_mday = atoi(in_buf);
        if (current_time.tm_mday > 31)
            current_time.tm_mday = 0;
    }
    while (current_time.tm_year < 0) {
        puts("Enter year: (1970-2038)");
        gets(in_buf);
        current_time.tm_year = atoi(in_buf) - 1900;
        if (current_time.tm_year > 138 ||
            current_time.tm_year < 70)
            current_time.tm_year = -1;
    }
    puts("Local time:\n");
    while (current_time.tm_hour < 0) {
        puts("Enter hour (0-23): ");
        gets(in_buf);
        current_time.tm_hour = atoi(in_buf);
        if (current_time.tm_hour > 23)
            current_time.tm_hour = -1;
    }
    while (current_time.tm_min < 0) {
        puts("Enter minute (0-59): ");
        gets(in_buf);
        current_time.tm_min = atoi(in_buf);
        if (current_time.tm_min > 59)
            current_time.tm_min = -1;
    }
    while (current_time.tm_sec < 0) {
        puts("Enter second (0-59): ");
        gets(in_buf);
        current_time.tm_sec = atoi(in_buf);
        if (current_time.tm_sec > 59)
            current_time.tm_sec = -1;
    }
    now.tv_sec = mktime(&current_time);
    now.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME, &now);
}
