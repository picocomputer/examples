/*
 * Copyright (c) 2026 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

/*
 * Multi-field form built on the readline editor.
 *
 * Demonstrates RIA_ATTR_RLN_LENGTH (per-field input cap; 0 for the
 * Submit "button"), RIA_ATTR_RLN_WIDTH locked to 80 (screen width --
 * not field width), and RIA_ATTR_RLN_HEIGHT left at default (the
 * attribute is screen height, also not field height; the form fits in
 * 24 rows).
 *
 * Navigation pattern: open CON: and poll. read() returns 0 while the
 * user types/edits, and returns the full newline-terminated line when
 * the editor flushes. Between reads, ria_readline_lastkey() reports
 * the last key with an `action` byte -- when action == 0 the editor
 * passed the key through (Tab, arrows, Shift-Tab) and we use it for
 * field navigation. To pull the buffer out without the user pressing
 * Enter, poke a CR; the editor then flushes through read() like any
 * other line.
 */

#include <rp6502.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define N_FIELDS   5
#define SUBMIT_IDX 5

#define NAV_PREV   1
#define NAV_NEXT   2
#define NAV_DONE   3
#define NAV_ABORT  4

static const char *labels[N_FIELDS] = {
    "Name:",
    "Email:",
    "SSN:",
    "Mother's maiden name:",
    "Name of first pet:",
};
static const unsigned char field_max[N_FIELDS] = { 30, 40, 11, 30, 30 };
static const unsigned char field_row[N_FIELDS] = { 7, 8, 9, 10, 11 };

#define FIELD_COL    32   /* absolute col where '[' sits */
#define INPUT_COL    33   /* first char position inside the brackets */
#define LABEL_COL     8

#define PANEL_COL     5
#define PANEL_ROW     2
#define PANEL_W      70
#define PANEL_H      14
#define TITLE_BOX_ROW 3
#define TITLE_BOX_COL 22
#define TITLE_BOX_W  32
#define TITLE_BOX_H   3
#define SUBMIT_ROW  13
#define SUBMIT_COL  34   /* leading '[' of "[  Submit  ]" */
#define SUBMIT_W    12

static char data[N_FIELDS][41];
static int con_fd;

static void draw_box(int row, int col, int w, int h, const char *sgr)
{
    int i, j;
    printf("%s\33[%d;%dH\33(0l", sgr, row, col);
    for (i = 0; i < w - 2; i++)
        putchar('q');
    putchar('k');
    for (j = 1; j < h - 1; j++)
    {
        printf("\33[%d;%dHx", row + j, col);
        printf("\33[%d;%dHx", row + j, col + w - 1);
    }
    printf("\33[%d;%dHm", row + h - 1, col);
    for (i = 0; i < w - 2; i++)
        putchar('q');
    printf("j\33(B\33[0m");
}

static void draw_field_strip(int idx)
{
    int j;
    /* Reset first so the leading '[' isn't tinted by any SGR the editor
     * left active. */
    printf("\33[0m\33[%d;%dH[\33[44m", field_row[idx], FIELD_COL);
    for (j = 0; j < field_max[idx]; j++)
        putchar(' ');
    printf("\33[0m]");
}

static void draw_static(void)
{
    int i;

    printf("\33[2J\33[H");

    draw_box(PANEL_ROW, PANEL_COL, PANEL_W, PANEL_H, "\33[36m");
    draw_box(TITLE_BOX_ROW, TITLE_BOX_COL, TITLE_BOX_W, TITLE_BOX_H, "\33[95m");

    /* Title text on a bright-white-on-blue strip (30 chars wide). */
    printf("\33[%d;%dH\33[1;97;44m     ## Ability de Gull ##    \33[0m",
           TITLE_BOX_ROW + 1, TITLE_BOX_COL + 1);

    for (i = 0; i < N_FIELDS; i++)
        draw_field_strip(i);
}

static void draw_labels(int active)
{
    int i;
    for (i = 0; i < N_FIELDS; i++)
    {
        printf("\33[%d;%dH", field_row[i], LABEL_COL);
        if (i == active)
            printf("\33[93m%s\33[0m", labels[i]);
        else
            printf("\33[0m%s", labels[i]);
    }
    printf("\33[%d;%dH", SUBMIT_ROW, SUBMIT_COL);
    if (active == SUBMIT_IDX)
        printf("\33[1;30;103m[  Submit  ]\33[0m");
    else
        printf("\33[1;97;42m[  Submit  ]\33[0m");
}

static int decode_nav(const char *key, int klen)
{
    if (klen == 1 && key[0] == '\t')
        return NAV_NEXT;
    if (klen == 3 && key[0] == 0x1B && key[1] == '[')
    {
        if (key[2] == 'A')
            return NAV_PREV;
        if (key[2] == 'B')
            return NAV_NEXT;
        if (key[2] == 'Z')
            return NAV_PREV;
    }
    return 0;
}

static int run_field(int idx, bool is_submit)
{
    bool first = true;
    int planned_nav = is_submit ? NAV_DONE : NAV_NEXT;
    char rdbuf[64];
    char key[33];
    unsigned char action;
    int n, klen, nav;
    int row, col;

    row = is_submit ? SUBMIT_ROW : field_row[idx];
    col = is_submit ? (SUBMIT_COL + SUBMIT_W) : INPUT_COL;

    ria_attr_set(is_submit ? 0 : field_max[idx], RIA_ATTR_RLN_LENGTH);

    for (;;)
    {
        if (first)
        {
            /* Position cursor at the field start and set SGR so the
             * editor's writes inherit white-on-blue. */
            printf("\33[0m\33[%d;%dH\33[97;44m", row, col);
        }

        n = read(con_fd, rdbuf, (int)sizeof(rdbuf));

        if (first)
        {
            /* The first read sets up the readline session at the current
             * cursor position. Only after that can we poke the saved
             * data into the editor -- poke writes the buffer back to
             * the screen, so this also restores the visible text. */
            if (!is_submit)
                ria_rln_poke(data[idx]);
            first = false;
        }

        if (n > 0)
        {
            if (!is_submit)
            {
                int end = n;
                while (end > 0 && (rdbuf[end - 1] == '\r' || rdbuf[end - 1] == '\n'))
                    end--;
                if (end > (int)field_max[idx])
                    end = field_max[idx];
                memcpy(data[idx], rdbuf, end);
                data[idx][end] = '\0';
            }
            printf("\33[0m");
            return planned_nav;
        }

        klen = ria_rln_lastkey(key, &action);
        if (klen > 0 && action == 0)
        {
            nav = decode_nav(key, klen);
            if (nav != 0)
            {
                planned_nav = nav;
                ria_rln_poke("\r");
            }
        }

        if (ria_attr_get(RIA_ATTR_SIGINT))
        {
            printf("\33[0m");
            return NAV_ABORT;
        }
    }
}

static int run_form(void)
{
    int cur = 0;
    int nav;

    ria_attr_set(80, RIA_ATTR_RLN_WIDTH);
    (void)ria_attr_get(RIA_ATTR_SIGINT); /* clear any pending latch */

    con_fd = open("CON:", 0);
    if (con_fd < 0)
    {
        printf("CON: not found.\n");
        return NAV_ABORT;
    }

    draw_static();

    for (;;)
    {
        draw_labels(cur);
        nav = run_field(cur, cur == SUBMIT_IDX);
        if (nav == NAV_DONE || nav == NAV_ABORT)
            break;
        if (nav == NAV_NEXT)
            cur = (cur >= SUBMIT_IDX) ? 0 : cur + 1;
        else if (nav == NAV_PREV)
            cur = (cur <= 0) ? SUBMIT_IDX : cur - 1;
    }

    close(con_fd);
    return nav;
}

static void show_result(int final_nav)
{
    int i;
    ria_attr_set(254, RIA_ATTR_RLN_LENGTH);
    printf("\33[2J\33[H\33[0m");
    if (final_nav == NAV_ABORT)
    {
        printf("Aborted (Ctrl-C).\n");
        return;
    }
    printf("Form submitted:\n\n");
    for (i = 0; i < N_FIELDS; i++)
        printf("  %-22s %s\n", labels[i], data[i]);
    printf("\n");
}

void main(void)
{
    int final_nav = run_form();
    show_result(final_nav);
}
