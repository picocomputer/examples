/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdlib.h>

// An example terminal app for the RP6502-RIA-W modem.
// Uses the ANSI terminal built in to the Pico VGA.

// Up to 512 bytes are needed for argv (one xstack size).
// Applications must opt-in to argc/argv by providing this memory.
void *__fastcall__ argv_mem(size_t size) { return malloc(size); }

// RIA.tx and RIA.rx have no canonical mode or newline translation.
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
    int fd, cp;

    const char *device = "AT:";
    if (argc == 2)
        device = argv[1];

    if (argc > 2)
    {
        print("Argument error.\r\n");
        return;
    }

    cp = code_page(437);
    if (cp != 437)
    {
        print("Code page 437 not found.\r\n");
    }

    fd = open(device, 0);
    if (fd < 0)
    {
        print("Modem not found.\r\n");
        return;
    }
    print("Modem online.\r\n");

    // The argv memory can be reclaimed
    free(argv);

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
