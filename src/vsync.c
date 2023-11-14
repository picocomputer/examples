/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define TIMEOUT 2048

unsigned irq_count;
uint8_t irq_stack[256];

unsigned char irq_fn(void)
{
    ++irq_count;
    RIA.irq = 1;
    return IRQ_HANDLED;
}

void main()
{
    unsigned i, j, k;
    static uint8_t v;

    set_irq(irq_fn, &irq_stack, sizeof(irq_stack));
    RIA.irq = 1;

    printf("Testing RIA_VSYNC and RIA_IRQ for 5 seconds.\nPlease wait.\n");

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
                        printf("SEQUENCE ERROR\n %d %d\n", v, RIA.vsync);
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

    RIA.irq = 0;

    printf("RIA_VSYNC PASS\n");

    if (irq_count >= 299 && irq_count <= 301)
        printf("RIA_IRQ PASS\n");
    else
        printf("RIA_IRQ count: %d\n", irq_count);
}
