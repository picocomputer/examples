#include <rp6502.h>
#include <clock.h>
#include <stdio.h>
#include <errno.h>

void test_time()
{
    time_t now;
    unsigned long res;
    res = time(&now);
    if (res == -1)
    {
        printf("errno: %d\n", errno);
        printf("_oserror: %d\n", _oserror);
    } else 
    {
        printf("%s", ctime(&now));

    }
}

void test_clock_getres()
{
    struct timespec resolution;
    unsigned long res;
    res = clock_getres(CLOCK_REALTIME, &resolution);
    if (res == -1)
    {
        printf("errno: %d\n", errno);
        printf("_oserror: %d\n", _oserror);
    } else 
    {
        printf("Clock resolution: seconds: %lu, nanoseconds: %lu\n", resolution.tv_sec, resolution.tv_nsec);
    }
}

void test_clock_gettime()
{
    struct timespec now;
    unsigned long res;
    struct tm time_struct;
    char buf[80];
    res = clock_gettime(CLOCK_REALTIME, &now);
    if (res == -1)
    {
        printf("errno: %d\n", errno);
        printf("_oserror: %d\n", _oserror);
    } else 
    {
        time_struct = *localtime(&(now.tv_sec));
        strftime(buf, sizeof(buf), "%a %m/%d/%Y %I:%M:%S %p", &time_struct);
        printf("%s\n", buf);
    }
}

void test_clock_settime()
{
    struct tm current_time;
    struct timespec now;
    current_time.tm_hour = 5;
    current_time.tm_sec = 5;
    current_time.tm_min = 5;
    current_time.tm_mday = 29;
    current_time.tm_mon = 10;
    current_time.tm_year = 123;
    current_time.tm_isdst = -1;
    current_time.tm_wday = 0;
    current_time.tm_yday = 0;
    now.tv_sec = mktime(&current_time);
    now.tv_nsec = 0;
    clock_settime(CLOCK_REALTIME, &now);
}

void main()
{
    printf("test clock_gettime...\n");
    test_clock_gettime();
    printf("test clock_settime...\n");
    test_clock_settime();
    printf("test clock_getres...\n");
    test_clock_getres();
    printf("test clock_gettime...\n");
    test_clock_gettime();
    printf("test time...\n");
    test_time();
}
