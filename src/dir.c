/*
 * Copyright (c) 2025 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define AM_RDO 0x01 /* Read only */
#define AM_HID 0x02 /* Hidden */
#define AM_SYS 0x04 /* System */
#define AM_DIR 0x10 /* Directory */
#define AM_ARC 0x20 /* Archive */

void format_attributes(unsigned char fattrib, char *buf)
{
    buf[0] = (fattrib & AM_RDO) ? 'R' : '-';
    buf[1] = (fattrib & AM_HID) ? 'H' : '-';
    buf[2] = (fattrib & AM_SYS) ? 'S' : '-';
    buf[3] = (fattrib & AM_DIR) ? 'D' : '-';
    buf[4] = (fattrib & AM_ARC) ? 'A' : '-';
    buf[5] = '\0';
}

void format_date_time(unsigned short fdate, unsigned short ftime, char *buf)
{
    int day = fdate & 0x1F;
    int month = (fdate >> 5) & 0x0F;
    int year = ((fdate >> 9) & 0x7F) + 1980;
    int seconds = (ftime & 0x1F) * 2;
    int minutes = (ftime >> 5) & 0x3F;
    int hours = (ftime >> 11) & 0x1F;
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hours, minutes, seconds);
}

void print_fileinfo(f_stat_t *fileinfo)
{
    char attr[6];
    char datetime[20];
    format_attributes(fileinfo->fattrib, attr);
    format_date_time(fileinfo->fdate, fileinfo->ftime, datetime);
    printf("%10ld %s %s %s\n", fileinfo->fsize, attr, datetime, fileinfo->fname);
}

void err(void)
{
    fprintf(stderr, "ERROR: %s\n", strerror(errno));
    exit(1);
}

void dir(char *path)
{
    f_stat_t *fileinfo;
    int dirdes;
    fileinfo = malloc(sizeof(fileinfo));
    dirdes = f_opendir(path);
    if (0 > dirdes)
        err();
    while (1)
    {
        if (0 > f_readdir(fileinfo, dirdes))
            err();
        if (!fileinfo->fname[0])
            break;
        print_fileinfo(fileinfo);
    }
    if (0 > f_closedir(dirdes))
        err();
    free(fileinfo);
}

void main()
{
    char s[256];
    unsigned long free_blocks, total_blocks;

    if (0 > f_getlabel("", &s))
        err();
    printf("LABEL: %s\n", s);

    if (0 > f_getcwd(&s, sizeof(s)))
        err();
    printf("PATH : %s\n", s);

    dir("");

    if (0 > f_getfree(&s, &free_blocks, &total_blocks))
        err();
    printf("FREE: %lu of %lu 512 byte blocks\n", free_blocks, total_blocks);
}
