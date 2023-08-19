#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>

void main()
{
    unsigned i, j, k;
    static uint8_t v;

    printf("Testing RIA_VSYNC for 5 seconds.\nPlease wait.\n");

    v = RIA_VSYNC;
    for (k = 0; k < 5; k++) // 5 seconds
    {
        for (j = 0; j < 60; j++) // 60 frames per second
        {
            for (i = 0; i < 2048; i++) // PHI2 based timeout
            {
                if (v != RIA_VSYNC)
                {
                    if ((uint8_t)(v + 1) != RIA_VSYNC)
                    {
                        printf("SEQUENCE ERROR\n");
                        exit(1);
                    }
                    v = RIA_VSYNC;
                    break;
                }
            }
            if (i == 1024)
            {
                printf("TIMEOUT ERROR\n");
                exit(1);
            }
        }
    }
    printf("PASS\n");
}
