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
#include <stdio.h>
#include <string.h>

// An example terminal app for the RP6502-RIA-W modem.
// Uses the ANSI terminal built in to the Pico VGA.

// Up to 512 bytes are needed for argv (one xstack size).
// Applications must opt-in to argc/argv by providing this memory.
// With malloc, you can free(argv) after using it.
void *__fastcall__ argv_mem(size_t size) { return malloc(size); }

int main(int argc, char *argv[])
{
    int fd_std, fd_dev, i;
    char rx_buf[32], tx_buf[32];
    unsigned char rx_buf_len = 0, tx_buf_len = 0;

    const char *device = "AT:";
    if (argc == 2)
        device = argv[1];

    if (argc > 2)
    {
        printf("Argument error.\n");
        return 1;
    }

    ria_attr_set(437, RIA_ATTR_CODE_PAGE);
    if (ria_attr_get(RIA_ATTR_CODE_PAGE) != 437)
    {
        printf("Code page 437 not found.\n");
        return 1;
    }

    fd_std = open("TTY:", 0);
    if (fd_std < 0)
    {
        printf("TTY: not found.\n");
        return 1;
    }

    fd_dev = open(device, 0);
    if (fd_dev < 0)
    {
        printf("%s not found.\n", device);
        return 1;
    }

    printf("Modem online.\n");

    while (true)
    {
        if (rx_buf_len)
        {
            i = write_xstack(rx_buf, rx_buf_len, fd_std);
            if (i < 0)
                goto fail;
            memmove(rx_buf, rx_buf + i, rx_buf_len - i);
            rx_buf_len -= i;
        }

        if (rx_buf_len < sizeof(rx_buf))
        {
            i = read_xstack(rx_buf + rx_buf_len, sizeof(rx_buf) - rx_buf_len, fd_dev);
            if (i < 0)
                goto fail;
            rx_buf_len += i;
        }

        if (tx_buf_len)
        {
            i = write_xstack(tx_buf, tx_buf_len, fd_dev);
            if (i < 0)
                goto fail;
            memmove(tx_buf, tx_buf + i, tx_buf_len - i);
            tx_buf_len -= i;
        }

        if (tx_buf_len < sizeof(tx_buf))
        {
            i = read_xstack(tx_buf + tx_buf_len, sizeof(tx_buf) - tx_buf_len, fd_std);
            if (i < 0)
                goto fail;
            tx_buf_len += i;
        }
    }

fail:
    printf("Unpossible IO error.\n");
    return 1;
}
