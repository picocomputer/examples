/*
 * Copyright (c) 2026 Rumbledethumps
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
#include <unistd.h>

#define NFC_CMD_WRITE 0x01
#define NFC_CMD_CANCEL 0x02
#define NFC_CMD_READ 0x03
#define NFC_CMD_SUCCESS1 0x04
#define NFC_CMD_SUCCESS2 0x05
#define NFC_CMD_ERROR 0x06

#define NFC_RESP_NO_READER 0x01
#define NFC_RESP_NO_CARD 0x02
#define NFC_RESP_CARD_INSERTED 0x03
#define NFC_RESP_CARD_READY 0x04
#define NFC_RESP_WRITE 0x05
#define NFC_RESP_READ 0x06

// Up to 512 bytes are needed for argv (one xstack size).
// Applications must opt-in to argc/argv by providing this memory.
void *__fastcall__ argv_mem(size_t size) { return malloc(size); }

static void print_help(void)
{
    char buf[64];
    int fd, n;
    fd = open("ROM:help", O_RDONLY);
    if (fd < 0)
    {
        puts("nfc: -r to read, -w <text> to write");
        return;
    }
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, n);
    close(fd);
}

static void print_hex(const unsigned char *data, unsigned len)
{
    unsigned i, j;
    for (i = 0; i < len; i += 16)
    {
        printf("%04X  ", i);
        for (j = 0; j < 16; j++)
        {
            if (j == 8)
                putchar(' ');
            if (i + j < len)
                printf("%02X ", data[i + j]);
            else
                printf("   ");
        }
        printf(" |");
        for (j = 0; j < 16 && i + j < len; j++)
        {
            unsigned char c = data[i + j];
            putchar((c >= 32 && c <= 126) ? c : '.');
        }
        puts("|");
    }
}

// Read exactly count bytes from fd. Returns 0 on success, -1 on error.
static int read_exact(int fd, unsigned char *buf, unsigned count)
{
    int n;
    while (count)
    {
        n = read(fd, buf, count);
        if (n < 0)
            return -1;
        if (n > 0)
        {
            buf += n;
            count -= n;
        }
    }
    return 0;
}

static void write_exact(int fd, const unsigned char *buf, unsigned count)
{
    int n;
    while (count)
    {
        n = write(fd, buf, count);
        if (n > 0)
        {
            buf += n;
            count -= n;
        }
    }
}

static void send_cmd(int fd, unsigned char cmd)
{
    write_exact(fd, &cmd, 1);
}

static const char *const uri_prefixes[] = {
    "", "http://www.", "https://www.", "http://", "https://",
    "tel:", "mailto:"};
#define URI_PREFIX_COUNT 7

static void print_safe(const unsigned char *s, unsigned len)
{
    unsigned i;
    for (i = 0; i < len; i++)
        putchar((s[i] >= 32 && s[i] <= 126) ? s[i] : '?');
    putchar('\n');
}

static void print_hex_bytes(const unsigned char *s, unsigned len)
{
    unsigned i;
    for (i = 0; i < len; i++)
        printf(" %02X", s[i]);
    putchar('\n');
}

// Returns bytes consumed, or 0 on error.
static unsigned print_record(const unsigned char *buf, unsigned avail)
{
    unsigned char flags, tnf, type_len, id_len;
    unsigned payload_len, hdr, aux;
    const unsigned char *tp;
    const unsigned char *pp;

    if (avail < 3)
        return 0;
    flags = buf[0];
    tnf = flags & 0x07;
    type_len = buf[1];
    hdr = 3;
    if (flags & 0x10)
        payload_len = buf[2];
    else
    {
        if (avail < 6)
            return 0;
        payload_len = ((unsigned)buf[4] << 8) | buf[5];
        hdr = 6;
    }
    id_len = 0;
    if (flags & 0x08)
    {
        id_len = buf[hdr];
        hdr++;
    }
    if (hdr + type_len + id_len + payload_len > avail)
        return 0;
    tp = buf + hdr;
    pp = buf + hdr + type_len + id_len;

    if (tnf == 0x01 && type_len == 1 && tp[0] == 'T' && payload_len >= 1)
    {
        aux = pp[0] & 0x3F; // language length
        printf("Text: ");
        print_safe(pp + 1 + aux, payload_len - 1 - aux);
    }
    else if (tnf == 0x01 && type_len == 1 && tp[0] == 'U' && payload_len >= 1)
    {
        aux = pp[0]; // URI prefix code
        printf("URI: ");
        if (aux < URI_PREFIX_COUNT)
            printf("%s", uri_prefixes[aux]);
        print_safe(pp + 1, payload_len - 1);
    }
    else
    {
        printf("Type:");
        print_hex_bytes(tp, type_len);
        printf("Data:");
        print_hex_bytes(pp, payload_len);
    }
    return hdr + type_len + id_len + payload_len;
}

static void decode_tlv(const unsigned char *buf, unsigned len)
{
    unsigned pos, tlen, end, n;
    unsigned char tag;

    pos = 0;
    while (pos < len)
    {
        tag = buf[pos++];
        if (tag == 0x00)
            continue;
        if (tag == 0xFE || pos >= len)
            break;
        tlen = buf[pos++];
        if (pos + tlen > len)
            break;
        end = pos + tlen;
        if (tag == 0x03)
            while (pos < end && (n = print_record(buf + pos, end - pos)))
                pos += n;
        pos = end;
    }
}

static void handle_read(int fd)
{
    unsigned char hdr[2];
    unsigned char *nfcbuf;
    unsigned len, i;

    send_cmd(fd, NFC_CMD_SUCCESS1);
    send_cmd(fd, NFC_CMD_SUCCESS2);

    if (read_exact(fd, hdr, 2) < 0)
    {
        puts("Read header failed");
        return;
    }

    // raw tag data from page 0: pages 0-2=UID/lock, page 3=CC, pages 4+=user data
    len = hdr[0] | ((unsigned)hdr[1] << 8);
    if (len)
    {
        nfcbuf = malloc(len);
        if (!nfcbuf)
        {
            puts("Out of memory");
            return;
        }
        if (read_exact(fd, nfcbuf, len) < 0)
        {
            puts("Read failed");
            free(nfcbuf);
            return;
        }
        print_hex(nfcbuf, len);
        // user data starts at page 4 (offset 16)
        if (len > 16)
            decode_tlv(nfcbuf + 16, len - 16);
        free(nfcbuf);
    }
}

static unsigned char *nfcbuf;
static unsigned nfcbuf_len;
static bool continuous;

// Build NDEF Text Record from argv[2..argc-1].
// Returns 0 on success, 1 on error (message already printed).
static int build_ndef(int argc, char *argv[])
{
    static char text[256];
    unsigned tlen, plen, i;

    if (argc < 3)
    {
        puts("nfc: -w requires text argument");
        return 1;
    }

    // Combine argv[2..] with spaces, quoting elements that need it
    tlen = 0;
    for (i = 2; i < (unsigned)argc; i++)
    {
        const char *arg = argv[i];
        unsigned char ch;
        bool needs_quotes = false;
        unsigned j;
        for (j = 0; arg[j]; j++)
        {
            ch = (unsigned char)arg[j];
            if (ch == ' ' || ch < 32 || ch > 126)
            {
                needs_quotes = true;
                break;
            }
        }
        if (tlen)
            text[tlen++] = ' ';
        if (needs_quotes)
            text[tlen++] = '"';
        for (j = 0; arg[j]; j++)
        {
            ch = (unsigned char)arg[j];
            if (tlen >= sizeof(text) - 6)
            {
                puts("Text too long");
                return 1;
            }
            if (ch < 32 || ch > 126)
            {
                text[tlen++] = '\\';
                text[tlen++] = '0' + (ch >> 6);
                text[tlen++] = '0' + ((ch >> 3) & 7);
                text[tlen++] = '0' + (ch & 7);
            }
            else
            {
                if (ch == '"' || ch == '\\')
                    text[tlen++] = '\\';
                text[tlen++] = ch;
            }
        }
        if (needs_quotes)
            text[tlen++] = '"';
    }

    // TLV: 03 <rec_len> D1 01 <plen> 54 02 'e' 'n' <text> FE
    plen = 3 + tlen; // NDEF payload: status + "en" + text
    if (plen > 255)
    {
        puts("Text too long for short record");
        return 1;
    }
    nfcbuf_len = 2 + (4 + plen) + 1; // TLV header + NDEF record + TLV terminator
    nfcbuf = malloc(nfcbuf_len);
    if (!nfcbuf)
    {
        puts("Out of memory");
        return 1;
    }
    nfcbuf[0] = 0x03;     // NDEF TLV tag
    nfcbuf[1] = 4 + plen; // NDEF record length
    nfcbuf[2] = 0xD1;     // MB=1 ME=1 SR=1 TNF=1 (well-known)
    nfcbuf[3] = 0x01;     // type length
    nfcbuf[4] = plen;     // payload length
    nfcbuf[5] = 0x54;     // type 'T' (Text)
    nfcbuf[6] = 0x02;     // status: UTF-8, language length = 2
    nfcbuf[7] = 'e';
    nfcbuf[8] = 'n';
    memcpy(nfcbuf + 9, text, tlen);
    nfcbuf[9 + tlen] = 0xFE; // terminator TLV

    return 0;
}

static void arm_write(int fd)
{
    unsigned char whdr[4];
    // nfcbuf[9..] is the text payload (ASCII-safe by construction)
    printf("Write armed: %.*s\n", (int)(nfcbuf_len - 10), (char *)(nfcbuf + 9));
    whdr[0] = NFC_CMD_WRITE;
    whdr[1] = 4; // start page (page 4 = start of user data)
    whdr[2] = nfcbuf_len & 0xFF;
    whdr[3] = (nfcbuf_len >> 8) & 0xFF;
    write_exact(fd, whdr, 4);
    write_exact(fd, nfcbuf, nfcbuf_len);
}

int main(int argc, char *argv[])
{
    int fd, tty;
    bool writing, had_card;
    unsigned char last_floor, resp, key;

    if (argc < 2 || (strcmp(argv[1], "-r") && strcmp(argv[1], "-R") &&
                     strcmp(argv[1], "-w") && strcmp(argv[1], "-W")))
    {
        print_help();
        return (argc < 2) ? 0 : 1;
    }

    writing = !strcmp(argv[1], "-w") || !strcmp(argv[1], "-W");
    continuous = !strcmp(argv[1], "-W") || !strcmp(argv[1], "-R");
    if (writing && build_ndef(argc, argv))
        return 1;

    fd = open("NFC:", O_RDWR);
    if (fd < 0)
    {
        puts("NFC: device not found");
        free(nfcbuf);
        return 1;
    }

    tty = open("TTY:", O_RDONLY);

    puts("Press Ctrl+C to exit");

    had_card = false;
    while (1)
    {
        if (tty >= 0 && read(tty, &key, 1) == 1 && key == 3)
        {
            close(fd);
            close(tty);
            puts("Goodbye");
            return 0;
        }

        if (read(fd, &resp, 1) < 1)
            continue;

        switch (resp)
        {
        case NFC_RESP_NO_READER:
            puts("No NFC reader");
            break;

        case NFC_RESP_NO_CARD:
            if (had_card)
                puts("Card removed");
            if (writing)
                arm_write(fd);
            puts("Insert card...");
            break;

        case NFC_RESP_CARD_INSERTED:
            puts("Card detected");
            had_card = true;
            break;

        case NFC_RESP_CARD_READY:
            if (!writing)
                send_cmd(fd, NFC_CMD_READ);
            break;

        case NFC_RESP_READ:
            handle_read(fd);
            if (!continuous)
            {
                close(fd);
                return 0;
            }
            break;

        case NFC_RESP_WRITE:
            send_cmd(fd, NFC_CMD_SUCCESS1);
            send_cmd(fd, NFC_CMD_SUCCESS2);
            puts("Write complete");
            if (!continuous)
            {
                close(fd);
                return 0;
            }
            break;
        }
    }
}
