/*
 * Copyright (c) 2023 Rumbledethumps
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

// 100% AI generated

#include <rp6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_RUNNERS 64 // 1-1024
#define NUM_STARS 512  // 1-1024
#define STARS_PER_FRAME 16

#define NUM_STAR_TYPES 4
#define NUM_STAR_COLORS 8
#define NUM_FRAMES 16
#define FRAME_DELAY 4

/* XRAM addresses */
#define XRAM_STAR_BITMAPS 0x0000
#define XRAM_RUNNER_FRAMES 0x0020
#define XRAM_RUNNER_PALETTE 0x0820
#define XRAM_STAR_PALETTES 0x0840
/* Configs packed at top of 64K XRAM */
#define XRAM_RUNNER_CONFIG (0u - (unsigned)NUM_RUNNERS * 8u)
#define XRAM_STAR_CONFIG (XRAM_RUNNER_CONFIG - (unsigned)NUM_STARS * 8u)

#define COLOR_FROM_RGB5(r, g, b) \
    (((unsigned)(b) << 11) | ((unsigned)(g) << 6) | (unsigned)(r))
#define COLOR_ALPHA (1u << 5)

// clang-format off

/* Star bitmaps: 4 types, 8x8 1bpp (8 bytes each) */
static const unsigned char star_bitmaps[4][8] = {
    {0x10, 0x10, 0x10, 0x7E, 0x10, 0x10, 0x10, 0x00}, /* cross */
    {0x10, 0x28, 0x44, 0x82, 0x44, 0x28, 0x10, 0x00}, /* diamond */
    {0x10, 0x10, 0x28, 0xC6, 0x28, 0x10, 0x10, 0x00}, /* 4-point */
    {0x00, 0x00, 0x10, 0x38, 0x10, 0x00, 0x00, 0x00}, /* dot */
};

/* Runner upper body (rows 0-9, 10 rows x 8 bytes = 80 bytes) */
static const unsigned char body_top[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* row 0  empty */
    0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, /* row 1  hair */
    0x00, 0x00, 0x02, 0x11, 0x20, 0x00, 0x00, 0x00, /* row 2  face */
    0x00, 0x00, 0x02, 0x11, 0x20, 0x00, 0x00, 0x00, /* row 3  face */
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, /* row 4  neck */
    0x00, 0x00, 0x33, 0x33, 0x33, 0x00, 0x00, 0x00, /* row 5  shoulders */
    0x00, 0x00, 0x33, 0x63, 0x33, 0x00, 0x00, 0x00, /* row 6  chest */
    0x00, 0x00, 0x03, 0x33, 0x30, 0x00, 0x00, 0x00, /* row 7  lower shirt */
    0x00, 0x00, 0x04, 0x44, 0x40, 0x00, 0x00, 0x00, /* row 8  hips */
    0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, /* row 9  upper legs */
};
// clang-format on

/* Stride offsets for 16-frame run cycle (right leg) */
static const signed char stride[16] = {
    0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1};

/* Runner palette: 16 colors x 2 bytes */
static const unsigned runner_palette[16] = {
    0x0000,                                    /* 0  transparent */
    COLOR_FROM_RGB5(28, 20, 16) | COLOR_ALPHA, /* 1  skin */
    COLOR_FROM_RGB5(6, 4, 2) | COLOR_ALPHA,    /* 2  hair */
    COLOR_FROM_RGB5(4, 8, 24) | COLOR_ALPHA,   /* 3  shirt */
    COLOR_FROM_RGB5(2, 4, 16) | COLOR_ALPHA,   /* 4  pants */
    COLOR_FROM_RGB5(12, 8, 4) | COLOR_ALPHA,   /* 5  shoes */
    COLOR_FROM_RGB5(8, 14, 28) | COLOR_ALPHA,  /* 6  highlight */
    COLOR_FROM_RGB5(2, 2, 2) | COLOR_ALPHA,    /* 7  dark */
    0, 0, 0, 0, 0, 0, 0, 0                     /* 8-15 unused */
};

/* Star colors (cycled for NUM_STARS > NUM_STAR_COLORS) */
static const unsigned star_colors[NUM_STAR_COLORS] = {
    COLOR_FROM_RGB5(31, 0, 0) | COLOR_ALPHA,
    COLOR_FROM_RGB5(0, 31, 0) | COLOR_ALPHA,
    COLOR_FROM_RGB5(31, 31, 0) | COLOR_ALPHA,
    COLOR_FROM_RGB5(0, 31, 31) | COLOR_ALPHA,
    COLOR_FROM_RGB5(31, 0, 31) | COLOR_ALPHA,
    COLOR_FROM_RGB5(31, 31, 31) | COLOR_ALPHA,
    COLOR_FROM_RGB5(31, 16, 0) | COLOR_ALPHA,
    COLOR_FROM_RGB5(16, 31, 16) | COLOR_ALPHA,
};

/* Runtime state */
static struct
{
    int x, y;
} star_pos[NUM_STARS];

static struct
{
    int x, y;
    unsigned char frame_idx;
} runner[NUM_RUNNERS];

static int runner_dir;
static unsigned char frame_timer;

/* Set a 4bpp pixel in a zeroed row buffer */
static void set_pixel(unsigned char *buf, int col, unsigned char color)
{
    unsigned char idx;
    if (col < 0 || col >= 16)
        return;
    idx = col >> 1;
    if (col & 1)
        buf[idx] |= color;
    else
        buf[idx] |= (color << 4);
}

void main()
{
    unsigned u, c;
    unsigned char f, row, i;
    uint8_t v;
    unsigned star_idx;

    _randomize();

    /* === Upload star bitmaps === */
    RIA.addr0 = XRAM_STAR_BITMAPS;
    RIA.step0 = 1;
    for (c = 0; c < 4; c++)
        for (i = 0; i < 8; i++)
            RIA.rw0 = star_bitmaps[c][i];

    /* === Generate and upload runner frames === */
    for (f = 0; f < NUM_FRAMES; f++)
    {
        signed char s = stride[f];

        RIA.addr0 = XRAM_RUNNER_FRAMES + (unsigned)f * 128;
        RIA.step0 = 1;

        /* Upper body (rows 0-9) */
        for (u = 0; u < sizeof(body_top); u++)
            RIA.rw0 = body_top[u];

        /* Legs (rows 10-15) */
        for (row = 10; row <= 15; row++)
        {
            unsigned char row_buf[8];
            int t, right_col, left_col;

            for (i = 0; i < 8; i++)
                row_buf[i] = 0;

            if (row <= 12)
            {
                t = row - 8;
                right_col = 7 + (int)s * t / 5;
                left_col = 6 - (int)s * t / 5;
                set_pixel(row_buf, right_col, 4);
                set_pixel(row_buf, left_col, 4);
            }
            else if (row == 13)
            {
                right_col = 7 + s;
                left_col = 6 - s;
                set_pixel(row_buf, right_col, 5);
                set_pixel(row_buf, left_col, 5);
            }
            else if (row == 14)
            {
                right_col = 7 + s;
                left_col = 6 - s;
                set_pixel(row_buf, right_col, 5);
                set_pixel(row_buf, right_col + 1, 5);
                set_pixel(row_buf, left_col, 5);
                set_pixel(row_buf, left_col + 1, 5);
            }
            /* row 15: stays empty */

            for (i = 0; i < 8; i++)
                RIA.rw0 = row_buf[i];
        }
    }

    /* === Upload runner palette === */
    RIA.addr0 = XRAM_RUNNER_PALETTE;
    RIA.step0 = 1;
    for (u = 0; u < 16; u++)
    {
        RIA.rw0 = runner_palette[u] & 0xFF;
        RIA.rw0 = (runner_palette[u] >> 8) & 0xFF;
    }

    /* === Upload star palettes (2 colors each) === */
    RIA.addr0 = XRAM_STAR_PALETTES;
    RIA.step0 = 1;
    for (c = 0; c < NUM_STARS; c++)
    {
        RIA.rw0 = 0;
        RIA.rw0 = 0;
        RIA.rw0 = star_colors[c % NUM_STAR_COLORS] & 0xFF;
        RIA.rw0 = (star_colors[c % NUM_STAR_COLORS] >> 8) & 0xFF;
    }

    /* === Initialize star sprite configs === */
    for (c = 0; c < NUM_STARS; c++)
    {
        unsigned x = ((uint32_t)rand() * 312) >> 15;
        unsigned y = ((uint32_t)rand() * 232) >> 15;
        star_pos[c].x = x;
        star_pos[c].y = y;

        RIA.addr0 = XRAM_STAR_CONFIG + c * 8;
        RIA.step0 = 1;
        RIA.rw0 = x & 0xFF;
        RIA.rw0 = (x >> 8) & 0xFF;
        RIA.rw0 = y & 0xFF;
        RIA.rw0 = (y >> 8) & 0xFF;
        u = XRAM_STAR_BITMAPS + (c % NUM_STAR_TYPES) * 8;
        RIA.rw0 = u & 0xFF;
        RIA.rw0 = (u >> 8) & 0xFF;
        u = XRAM_STAR_PALETTES + c * 4;
        RIA.rw0 = u & 0xFF;
        RIA.rw0 = (u >> 8) & 0xFF;
    }

    /* === Initialize runner sprite configs === */
    runner_dir = 1;
    frame_timer = 0;
    for (c = 0; c < NUM_RUNNERS; c++)
    {
        runner[c].x = ((uint32_t)rand() * 320) >> 15;
        runner[c].y = (uint32_t)c * 240 / NUM_RUNNERS;
        runner[c].frame_idx = rand() % NUM_FRAMES;

        RIA.addr0 = XRAM_RUNNER_CONFIG + c * 8;
        RIA.step0 = 1;
        RIA.rw0 = runner[c].x & 0xFF;
        RIA.rw0 = (runner[c].x >> 8) & 0xFF;
        RIA.rw0 = runner[c].y & 0xFF;
        RIA.rw0 = (runner[c].y >> 8) & 0xFF;
        u = XRAM_RUNNER_FRAMES + (unsigned)runner[c].frame_idx * 128;
        RIA.rw0 = u & 0xFF;
        RIA.rw0 = (u >> 8) & 0xFF;
        u = XRAM_RUNNER_PALETTE;
        RIA.rw0 = u & 0xFF;
        RIA.rw0 = (u >> 8) & 0xFF;
    }

    /* === Program VGA === */
    xreg_vga_canvas(1);
    xreg_vga_mode(0, 1);
    xreg_vga_mode(5, 0x00, XRAM_STAR_CONFIG, NUM_STARS, 0);
    xreg_vga_mode(5, 0x0A, XRAM_RUNNER_CONFIG, NUM_RUNNERS, 1);

    /* === Main loop === */
    v = RIA.vsync;
    star_idx = 0;

    while (1)
    {
        if (RIA.vsync == v)
            continue;
        v = RIA.vsync;

        /* Write star positions (optimized dual-port) */
        RIA.step0 = 8;
        RIA.step1 = 8;
        RIA.addr0 = XRAM_STAR_CONFIG;
        RIA.addr1 = XRAM_STAR_CONFIG + 1;
        for (c = 0; c < NUM_STARS; c++)
        {
            int val = star_pos[c].x;
            RIA.rw0 = val & 0xFF;
            RIA.rw1 = (val >> 8) & 0xFF;
        }
        RIA.addr0 = XRAM_STAR_CONFIG + 2;
        RIA.addr1 = XRAM_STAR_CONFIG + 3;
        for (c = 0; c < NUM_STARS; c++)
        {
            int val = star_pos[c].y;
            RIA.rw0 = val & 0xFF;
            RIA.rw1 = (val >> 8) & 0xFF;
        }

        /* Write runner positions (optimized dual-port) */
        RIA.step0 = 8;
        RIA.step1 = 8;
        RIA.addr0 = XRAM_RUNNER_CONFIG;
        RIA.addr1 = XRAM_RUNNER_CONFIG + 1;
        for (c = 0; c < NUM_RUNNERS; c++)
        {
            int val = runner[c].x;
            RIA.rw0 = val & 0xFF;
            RIA.rw1 = (val >> 8) & 0xFF;
        }
        RIA.addr0 = XRAM_RUNNER_CONFIG + 4;
        RIA.addr1 = XRAM_RUNNER_CONFIG + 5;
        for (c = 0; c < NUM_RUNNERS; c++)
        {
            u = XRAM_RUNNER_FRAMES + (unsigned)runner[c].frame_idx * 128;
            RIA.rw0 = u & 0xFF;
            RIA.rw1 = (u >> 8) & 0xFF;
        }

        /* Animate and move runners */
        if (++frame_timer >= FRAME_DELAY)
        {
            frame_timer = 0;
            for (c = 0; c < NUM_RUNNERS; c++)
            {
                runner[c].frame_idx = (runner[c].frame_idx + 1) % NUM_FRAMES;
                runner[c].x += runner_dir;
                if (runner[c].x > 320)
                    runner[c].x = -16;
                if (runner[c].x < -16)
                    runner[c].x = 320;
            }
        }

        /* Move stars opposite to runner */
        for (i = 0; i < STARS_PER_FRAME; i++)
        {
            star_pos[star_idx].x -= runner_dir;
            if (star_pos[star_idx].x < -8)
                star_pos[star_idx].x = 319;
            if (star_pos[star_idx].x > 319)
                star_pos[star_idx].x = -8;
            star_idx = (star_idx + 1) % NUM_STARS;
        }
    }
}
