#include <rp6502.h>
#include <clock.h>
#include <stdio.h>
#include <errno.h>

void main()
{
    time_t now;
    unsigned long res;
    struct tm time_struct;
    char buf[80];
    res = time(&now);
    if (res == -1)
    {
        printf("errno: %d\n", errno);
        printf("_oserror: %d\n", _oserror);
    } else 
    {
        time_struct = *localtime(&now);
        strftime(buf, sizeof(buf), "%a %m/%d/%Y %I:%M:%S %p", &time_struct);
        printf("%s\n", buf);
    }
}
