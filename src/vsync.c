#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define TIMEOUT 2048

void main()
{
    unsigned i, j, k;
    static uint8_t v;

    printf("Testing RIA_VSYNC for 5 seconds.\nPlease wait.\n");

    v = RIA.vsync;
    for (k = 0; k < 5; k++) // 5 seconds
    {
        for (j = 0; j < 60; j++) // 60 frames per second
        {
            for (i = 0; i < TIMEOUT; i++) // PHI2 based timeout
            {
                if (v != RIA.vsync)
                {
                    if ((uint8_t)(v + 1) != RIA.vsync)
                    {
                        printf("SEQUENCE ERROR\n %d %d\n",v, RIA.vsync);
                        exit(1);
                    }
                    v = RIA.vsync;
                    break;
                }
            }
            if (i == TIMEOUT)
            {
                printf("TIMEOUT ERROR\n");
                exit(1);
            }
        }
    }
    printf("PASS\n");
}
