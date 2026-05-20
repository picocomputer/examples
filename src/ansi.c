/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 80
#define MAX_FAILS 16

static int tty;
static int term_h = 30;
static unsigned pass_count;
static unsigned fail_count;

static const char *fail_label[MAX_FAILS];
static int         fail_gy[MAX_FAILS];
static int         fail_gx[MAX_FAILS];
static int         fail_ey[MAX_FAILS];
static int         fail_ex[MAX_FAILS];

static char pause_buf[8];

static void cls(void)
{
    printf("\33[2J\33[H");
    fflush(stdout);
}

static void pause_page(const char *prompt)
{
    printf("%s", prompt);
    fflush(stdout);
    gets(pause_buf);
}

/* Like pause_page but reads CR directly from tty so readline doesn't
 * reset DECTCEM/DECSCUSR state. Prints "X\b" so the cursor sits over
 * a visible glyph while the user inspects its appearance. */
static void pause_tty(const char *prompt)
{
    unsigned char b;
    int r;
    printf("%sX\b", prompt);
    fflush(stdout);
    for (;;)
    {
        r = read(tty, &b, 1);
        if (r < 0)
            return;
        if (r == 0)
            continue;
        if (b == '\r' || b == '\n')
            break;
    }
    printf("\n");
    fflush(stdout);
}

/* Read CPR (\33[y;xR) following a \33[6n query. */
static int query_cpr(int *y, int *x)
{
    unsigned char buf[16];
    unsigned char b;
    unsigned n = 0;
    int r;

    printf("\33[6n");
    fflush(stdout);

    while (n < sizeof(buf) - 1)
    {
        r = read(tty, &b, 1);
        if (r < 0)
            return -1;
        if (r == 0)
            continue;
        buf[n++] = b;
        if (b == 'R')
            break;
    }
    buf[n] = 0;

    if (n < 4 || buf[0] != 0x1B || buf[1] != '[')
        return -1;
    if (sscanf((char *)buf + 2, "%d;%d", y, x) != 2)
        return -1;
    return 0;
}

static void record_fail(const char *label, int gy, int gx, int ey, int ex)
{
    if (fail_count < MAX_FAILS)
    {
        fail_label[fail_count] = label;
        fail_gy[fail_count] = gy;
        fail_gx[fail_count] = gx;
        fail_ey[fail_count] = ey;
        fail_ex[fail_count] = ex;
    }
    fail_count++;
}

static void expect_pos(int ey, int ex, const char *label)
{
    int gy = -1, gx = -1;
    if (query_cpr(&gy, &gx) == 0 && gy == ey && gx == ex)
        pass_count++;
    else
        record_fail(label, gy, gx, ey, ex);
}

static void expect_y(int ey, const char *label)
{
    int gy = -1, gx = -1;
    if (query_cpr(&gy, &gx) == 0 && gy == ey)
        pass_count++;
    else
        record_fail(label, gy, gx, ey, -1);
}

static void run_automated(void)
{
    int gy, gx;

    cls();
    printf("Running automated tests...\n");
    fflush(stdout);

    /* Height detect: clamp to terminal bottom. Width assumed 80. */
    printf("\33[999;1H");
    if (query_cpr(&gy, &gx) == 0)
    {
        term_h = gy;
        pass_count++;
    }
    else
    {
        record_fail("height detect", -1, -1, 0, 0);
    }

    /* Cursor positioning */
    printf("\33[5;10H");                 expect_pos(5, 10, "CUP");
    printf("\33[10;10H\33[3A");          expect_pos(7, 10, "CUU");
    printf("\33[5;10H\33[3B");           expect_pos(8, 10, "CUD");
    printf("\33[5;5H\33[3C");            expect_pos(5, 8,  "CUF");
    printf("\33[5;10H\33[3D");           expect_pos(5, 7,  "CUB");
    printf("\33[5;10H\33[2E");           expect_pos(7, 1,  "CNL");
    printf("\33[10;10H\33[2F");          expect_pos(8, 1,  "CPL");
    printf("\33[5;10H\33[40G");          expect_pos(5, 40, "CHA");
    printf("\33[5;10H\33[12d");          expect_pos(12, 10,"VPA");
    printf("\33[7;15f");                 expect_pos(7, 15, "HVP");

    /* Default-param behavior */
    printf("\33[H");                     expect_pos(1, 1,  "CUP default");
    printf("\33[5;10H\33[A");            expect_pos(4, 10, "CUU default 1");
    printf("\33[5;10H\33[B");            expect_pos(6, 10, "CUD default 1");
    printf("\33[5;10H\33[C");            expect_pos(5, 11, "CUF default 1");
    printf("\33[5;10H\33[D");            expect_pos(5, 9,  "CUB default 1");

    /* Edge clamping */
    printf("\33[1;1H\33[1A");            expect_pos(1, 1,  "CUU clamp top");
    printf("\33[5;1H\33[1D");            expect_pos(5, 1,  "CUB clamp left");
    printf("\33[%d;80H\33[1B", term_h);  expect_pos(term_h, 80, "CUD clamp bot");
    printf("\33[5;1H\33[200G");          expect_pos(5, WIDTH, "CHA clamp right");

    /* Tabs -- set explicit stops at 9,17,...,73 first (HTS at each column),
     * since the firmware may not initialize default 8-col tab stops. */
    printf("\33[3g");
    {
        int t;
        for (t = 9; t <= 73; t += 8)
            printf("\33[5;%dH\33H", t);
    }
    printf("\33[5;1H\33[I");             expect_pos(5, 9,  "CHT");
    printf("\33[5;17H\33[Z");            expect_pos(5, 9,  "CBT");
    printf("\33[5;1H\t");                expect_pos(5, 9,  "HT");

    /* C0 controls */
    printf("\33[5;10H\b");               expect_pos(5, 9,  "BS");
    printf("\33[5;10H\r");               expect_pos(5, 1,  "CR");
    printf("\33[5;10H\n");               expect_y(6,        "LF");

    /* Save / restore */
    printf("\33[8;12H\33[s\33[1;1H\33[u");        expect_pos(8, 12, "SCP/RCP");
    printf("\33[9;13H\0337\33[1;1H\0338");        expect_pos(9, 13, "DECSC/DECRC");

    /* Scroll region + origin mode: in origin mode \33[999;1H clamps to
     * region bottom, and CPR returns coords relative to margin_top. */
    printf("\33[5;15r\33[?6h\33[1;1H");           expect_pos(1, 1, "DECOM home");
    printf("\33[5;15r\33[?6h\33[999;1H");         expect_pos(11, 1, "DECOM+STBM clamp");
    printf("\33[?6l\33[r\33[1;1H");               expect_pos(1, 1, "DECOM off + STBM reset");

    /* Auto-wrap on: at (5,80) printing one char enters deferred wrap;
     * the next char wraps to (6,1) and advances to (6,2). */
    printf("\33[5;80HAB");                        expect_pos(6, 2, "DECAWM on");
    /* Auto-wrap off: cursor pinned at right margin. */
    printf("\33[?7l\33[5;80HAB");                 expect_pos(5, 80, "DECAWM off");
    printf("\33[?7h");

    /* Soft reset: homes cursor, clears scroll region and origin. */
    printf("\33[5;15r\33[?6h\33[10;5H\33[!p");    expect_pos(1, 1, "DECSTR");

    /* DECSTBM scroll behavior. LF at bottom margin keeps the cursor on
     * the bottom-margin row (the region scrolls up). RI (ESC M) at the
     * top margin keeps the cursor on the top-margin row (region scrolls
     * down). */
    printf("\33[5;15r\33[15;1H\n");               expect_y(15, "LF at bot margin");
    printf("\33[5;15r\33[5;1H\33M");              expect_y(5,  "RI at top margin");

    /* DECAWM inside DECSTBM: deferred wrap at the right margin on the
     * bottom-margin row -- the next char scrolls the region instead of
     * moving the cursor out, leaving it at (bot_margin, 2). */
    printf("\33[5;15r\33[?7h\33[15;80HAB");       expect_pos(15, 2, "DECAWM+STBM wrap");

    /* IL / DL within the scroll region: row is preserved. Column
     * behavior varies between DEC and ECMA-48, so check Y only. */
    printf("\33[5;15r\33[8;1H\33[2L");            expect_y(8, "IL in region");
    printf("\33[5;15r\33[8;1H\33[2M");            expect_y(8, "DL in region");

    /* Tidy state before phase 2 */
    printf("\33[r\33[?6l\33[?7h\33[0m");

    cls();
    printf("Automated tests: %u passed, %u failed\n", pass_count, fail_count);
    if (fail_count)
    {
        unsigned i;
        unsigned n = (fail_count < MAX_FAILS) ? fail_count : MAX_FAILS;
        for (i = 0; i < n; i++)
            printf("  FAIL %-22s got (%d,%d) want (%d,%d)\n",
                   fail_label[i], fail_gy[i], fail_gx[i],
                   fail_ey[i], fail_ex[i]);
        if (fail_count > MAX_FAILS)
            printf("  ... %u more\n", fail_count - MAX_FAILS);
        printf("\nNote: results are only valid when the VGA console is the\n");
        printf("only console. A remote console (telnet or serial) will\n");
        printf("override the CPR responses and cause spurious failures.\n");
    }
    printf("\n");
    pause_page("Enter to continue: ");
}

/* Attributes + DEC graphics share a screen. Bold/faint/italic use blue
 * (SGR 34) so the intensity difference is visible; on a white default fg
 * they all look identical. Underline / double-underline / overline rows
 * are spaced apart so their pixel decorations don't visually merge. */
static void visual_attributes_and_dec(void)
{
    int i;
    cls();
    printf("\33[1mText Attributes\33[0m   (bold/faint/italic shown in blue)\n");
    printf("Default        : \33[34mThe quick brown fox\33[0m\n");
    printf("Bold           : \33[1;34mThe quick brown fox 0123456789\33[0m\n");
    printf("Faint          : \33[2;34mThe quick brown fox 0123456789\33[0m\n");
    printf("Italic         : \33[3;34mThe quick brown fox 0123456789\33[0m\n");
    printf("\n");
    printf("Underline      : \33[4mThe quick brown fox 0123456789\33[0m\n");
    printf("\n");
    printf("Double under   : \33[21mThe quick brown fox 0123456789\33[0m\n");
    printf("\n");
    printf("Overline       : \33[53mThe quick brown fox 0123456789\33[0m\n");
    printf("\n");
    printf("Strikethrough  : \33[9mThe quick brown fox 0123456789\33[0m\n");
    printf("\n");
    printf("Reverse        : \33[7mThe quick brown fox 0123456789\33[0m\n");
    printf("Conceal        : \33[8mThe quick brown fox 0123456789\33[0m  (hidden)\n");
    printf("Blink          : \33[5mThe quick brown fox 0123456789\33[0m\n");

    printf("Bright fg/bg   : ");
    for (i = 90; i < 98; i++)
        printf("\33[%dm##", i);
    printf("\33[0m  ");
    for (i = 100; i < 108; i++)
        printf("\33[%dm  ", i);
    printf("\33[0m\n\n");

    printf("DEC graphics   : \33(0lqqqqqqqqqqqk\33(B\n");
    printf("                 \33(0x           x\33(B\n");
    printf("                 \33(0mqqqqqqqqqqqj\33(B\n");
    printf("\n");
    pause_page("Enter: ");
}

/* 16-color + 256-color cube + grayscale share a screen. */
static void visual_palette(void)
{
    int i, j;
    cls();
    printf("\33[1mIndexed palette\33[0m\n\n");

    printf("Standard  0-7  : ");
    for (i = 0; i < 8; i++)
        printf("\33[48;5;%dm      ", i);
    printf("\33[0m\n");

    printf("Bright    8-15 : ");
    for (i = 8; i < 16; i++)
        printf("\33[48;5;%dm      ", i);
    printf("\33[0m\n\n");

    printf("216 colors (6x6x6):\n");
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 36; j++)
            printf("\33[48;5;%dm  ", 16 + i * 36 + j);
        printf("\33[0m\n");
    }

    printf("\nGrayscale 232-255: ");
    for (i = 232; i < 256; i++)
        printf("\33[48;5;%dm  ", i);
    printf("\33[0m\n\n");
    printf("\33[1m24-bit RGB gradient\33[0m   (32 x 8)\n\n");

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 32; j++)
        {
            int r = i * 32;
            int g = j * 8;
            int b = r;
            printf("\33[48;2;%d;%d;%dm  ", r, g, b);
        }
        printf("\33[0m\n");
    }
    printf("\n");
    pause_page("Enter: ");
}

/* OSC default colors + ?1049 alt screen share a screen. */
static void visual_osc_and_alt(void)
{
    int i;
    cls();
    printf("\33[1mOSC 10/11/12 default colors\33[0m\n");
    printf("Default colors  : The quick brown fox\n");
    /* Terminate OSC with ST (ESC \) -- BEL would beep. */
    printf("\33]10;#88ff88\33\\\33]11;#222244\33\\\33]12;#ff0000\33\\");
    printf("Overridden      : The quick brown fox\n");
    printf("\33]110\33\\\33]111\33\\\33]112\33\\");
    printf("Reset           : The quick brown fox\n\n");

    printf("\33[1m?1049 alt screen buffer\33[0m\n");
    pause_page("Enter to switch to alt screen: ");

    printf("\33[?1049hALT SCREEN -- the primary should be hidden.\n\n");
    for (i = 1; i <= 8; i++)
        printf("  alt line %d\n", i);
    printf("\n");
    pause_page("Enter to return to primary: ");

    printf("\33[?1049l");
    pause_page("Primary restored -- Enter: ");
}

/* All cursor tests on one screen: DECTCEM visibility then DECSCUSR styles.
 * The cursor that matters in each case is the one drawn at the gets() prompt
 * while the mode is active. */
static void visual_cursor_tests(void)
{
    static const char *names[7] = {
        "0 host default",
        "1 blinking block",
        "2 steady block",
        "3 blinking underline",
        "4 steady underline",
        "5 blinking bar",
        "6 steady bar",
    };
    int ps;
    cls();
    printf("\33[1mCursor tests\33[0m  -- watch the cursor at each prompt\n\n");

    printf("DECTCEM visibility (?25):\n");
    pause_tty("  cursor visible -- Enter: ");
    printf("\33[?25l");
    pause_tty("  cursor hidden  -- Enter: ");
    printf("\33[?25h");

    printf("\nDECSCUSR styles (CSI Ps SP q):\n");
    for (ps = 0; ps < 7; ps++)
    {
        char prompt[64];
        printf("\33[%d q", ps);
        sprintf(prompt, "  style %s -- Enter: ", names[ps]);
        pause_tty(prompt);
    }
    printf("\33[0 q"); /* restore default style */
}

/* DECSTBM visual: lay down sentinel rows outside a 5..12 scroll region,
 * then walk the user through scroll-up via LF at bot margin, scroll-down
 * via RI at top margin, IL/DL inside the region, and the DECAWM+DECSTBM
 * right-margin wrap. Rows 1-4 and 13+ must stay still throughout. */
static void visual_decstbm_scroll(void)
{
    int i;
    cls();
    printf("\33[3;1HOutside top (fixed)");
    printf("\33[14;1HOutside bottom (fixed)");

    /* Bracket the region with DEC graphics horizontal rules on rows 4
     * and 13 so the boundary is visually unambiguous. */
    printf("\33[4;1H\33(0");
    for (i = 0; i < WIDTH; i++)
        putchar('q');
    printf("\33(B");
    printf("\33[13;1H\33(0");
    for (i = 0; i < WIDTH; i++)
        putchar('q');
    printf("\33(B");

    /* Region rows 5-12, fill with 8 lines of context. */
    printf("\33[5;12r");
    for (i = 1; i <= 8; i++)
        printf("\33[%d;1Hregion line %d", 4 + i, i);

    pause_page("\33[18;1H\33[2KEnter: scroll up via LF at bot margin (x4): ");
    for (i = 1; i <= 4; i++)
        printf("\33[12;1H\33[Knew bottom %d\n", i);

    pause_page("\33[18;1H\33[2KEnter: scroll down via RI at top margin (x4): ");
    for (i = 1; i <= 4; i++)
        printf("\33[5;1H\33M\33[Knew top %d", i);

    pause_page("\33[18;1H\33[2KEnter: IL inside region: ");
    printf("\33[8;1H\33[2L>>> inserted at row 8 <<<");

    pause_page("\33[18;1H\33[2KEnter: DL inside region: ");
    printf("\33[7;1H\33[2M");

    pause_page("\33[18;1H\33[2KEnter: DECAWM+DECSTBM right-margin wrap: ");
    printf("\33[12;76H1234567890ABCDE");

    printf("\33[r");
    pause_page("\33[18;1H\33[2KEnter to continue: ");
}

void main(void)
{
    tty = open("TTY:", O_RDONLY);
    if (tty < 0)
    {
        printf("TTY: not found\n");
        return;
    }

    run_automated();

    visual_attributes_and_dec();
    visual_palette();
    visual_osc_and_alt();
    visual_cursor_tests();
    visual_decstbm_scroll();

    /* final cleanup */
    printf("\33[?25h\33[0m\33[0 q");
    printf("\33]110\33\\\33]111\33\\\33]112\33\\");
    printf("\nansi test complete.\n");

    close(tty);
}
