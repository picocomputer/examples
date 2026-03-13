/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// argc/argv requires memory
void *__fastcall__ argv_mem(size_t size) { return malloc(size); }

// An extremely simple terminal for the Pico RIA W modem.
// Uses the terminal built in to the Pico VGA.

void print(char *s)
{
    while (*s)
        if (RIA.ready & RIA_READY_TX_BIT)
            RIA.tx = *s++;
}

void main(int argc, char *argv[])
{
    char rx_char, tx_char;
    bool rx_mode, tx_mode;
    int fd, cp, i;

    printf("argc: %d\n", argc);
    for (i = 0; i < argc; i++)
        printf("%d: %s\r\n", i, argv[i]);

    cp = code_page(437);
    if (cp != 437)
    {
        print("Code page 437 not found.\r\n");
    }

    fd = open("AT:", 0);
    if (fd < 0)
    {
        print("Modem not found.\r\n");
        return;
    }
    print("Modem online.\r\n");

    while (true)
    {
        if (!rx_mode)
        {
            ria_push_char(1);
            ria_set_ax(fd);
            rx_mode = ria_call_int(RIA_OP_READ_XSTACK);
            rx_char = ria_pop_char();
        }
        else if ((RIA.ready & RIA_READY_TX_BIT))
        {
            RIA.tx = rx_char;
            rx_mode = false;
        }

        if (tx_mode)
        {
            ria_push_char(tx_char);
            ria_set_ax(fd);
            tx_mode = !ria_call_int(RIA_OP_WRITE_XSTACK);
        }
        else if (RIA.ready & RIA_READY_RX_BIT)
        {
            tx_char = RIA.rx;
            tx_mode = true;
        }
    }
}
