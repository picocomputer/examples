/*
 * 3D Cube Animation — PRE-COMPUTED + MATH COPROCESSOR variant
 *
 * All vertex screen coordinates for all 180 animation frames
 * (angle = 0, 2, 4, ... 358 degrees) are computed once at startup
 * using the RP2350 math coprocessor (IEEE 754 fp32).
 * The animation loop reads only from those tables — no math per frame.
 *
 * Memory:
 *   pre_x[180][8] + pre_y[180][8] + pre_r[180][8]
 *   = 3 * 180 * 8 * 2 bytes = 8 640 bytes of RAM
 *
 * XRAM layout:
 *   0x0000 - 0x707F  Framebuffer A (28 800 bytes)
 *   0x7080 - 0xE0FF  Framebuffer B (28 800 bytes)
 *   0xFF00 - 0xFF0D  vga_mode3_config_t
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/* ---------- VGA framebuffer layout ---------- */

#define FB_WIDTH   640
#define FB_HEIGHT  360
#define FB_SIZE    28800u
#define FB_A       0x0000u
#define FB_B       0x7080u
#define CFG_ADDR   0xFF00u

#define CX  320
#define CY  180

/* ---------- Math coprocessor constants (IEEE 754 fp32_t) ---------- */

#define F32_PI_OVER_180  0x3C8EFA35UL  /* pi/180  ~= 0.017453 */
#define F32_SIN30        0x3F000000UL  /* sin(30 deg) = 0.5    */
#define F32_COS30        0x3F5DB3D7UL  /* cos(30 deg) ~= 0.866 */
#define F32_D            0x43480000UL  /* viewer distance = 200.0 */

/* Base circle radius in pixels — edit this value to change circle size */
#define BASE_CIRCLE_R    2

/* ---------- Animation parameters ---------- */

#define ANGLE_STEP   2
#define NUM_FRAMES   (360 / ANGLE_STEP)  /* 180 */

/* ---------- Cube geometry ---------- */

#define HALF 50

static const signed char verts[8][3] = {
    {-HALF, -HALF, -HALF},
    {+HALF, -HALF, -HALF},
    {+HALF, +HALF, -HALF},
    {-HALF, +HALF, -HALF},
    {-HALF, -HALF, +HALF},
    {+HALF, -HALF, +HALF},
    {+HALF, +HALF, +HALF},
    {-HALF, +HALF, +HALF},
};

static const unsigned char edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7},
};

static const unsigned char bitmask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

/* ---------- Pre-computed coordinate tables ---------- */

/*
 * 180 frames, 8 vertices each.
 * Total: 3 * 180 * 8 * 2 = 8 640 bytes.
 */
static int pre_x[NUM_FRAMES][8];
static int pre_y[NUM_FRAMES][8];
static int pre_r[NUM_FRAMES][8];

/* Compute screen coordinates for one frame using math coprocessor
 * and store results in the pre-computed tables. */
static void precompute_frame(int frame)
{
    int angle = frame * ANGLE_STEP;
    fp32_t fa, sin_a, cos_a, f_base_r;
    fp32_t fx, fy, fz, rx, ry, rz, sy, sz, w;
    int i;

    f_base_r = mth_itof(BASE_CIRCLE_R);

    fa    = mth_mulf(mth_itof(angle), F32_PI_OVER_180);
    sin_a = mth_sinf(fa);
    cos_a = mth_cosf(fa);

    for (i = 0; i < 8; i++)
    {
        fx = mth_itof((int)verts[i][0]);
        fy = mth_itof((int)verts[i][1]);
        fz = mth_itof((int)verts[i][2]);

        /* Ry(angle) */
        rx = mth_addf(mth_mulf(fx, cos_a), mth_mulf(fz, sin_a));
        ry = fy;
        rz = mth_subf(mth_mulf(fz, cos_a), mth_mulf(fx, sin_a));

        /* Rx(+30 deg) */
        sy = mth_subf(mth_mulf(ry, F32_COS30), mth_mulf(rz, F32_SIN30));
        sz = mth_addf(mth_mulf(ry, F32_SIN30), mth_mulf(rz, F32_COS30));

        /* Perspective: w = D / (D - sz) */
        w = mth_divf(F32_D, mth_subf(F32_D, sz));

        pre_x[frame][i] = CX + (int)mth_ftoi(mth_mulf(rx, w));
        pre_y[frame][i] = CY - (int)mth_ftoi(mth_mulf(sy, w));
        pre_r[frame][i] = (int)mth_ftoi(mth_mulf(f_base_r, w));
        if (pre_r[frame][i] < 1) pre_r[frame][i] = 1;
    }
}

/* ---------- XRAM base address of the back buffer ---------- */

static unsigned back_buf;

/* ---------- Pixel and line drawing ---------- */

static void set_pixel(int x, int y)
{
    unsigned addr;
    if ((unsigned)x >= (unsigned)FB_WIDTH || (unsigned)y >= (unsigned)FB_HEIGHT)
        return;
    addr = back_buf
         + (((unsigned)y << 6) + ((unsigned)y << 4))
         + ((unsigned)x >> 3);
    RIA.addr0 = addr;
    RIA.rw0 = RIA.rw0 | bitmask[x & 7];
}

static void draw_line(int x0, int y0, int x1, int y1)
{
    int dx, dy, adx, ady, sx, sy, err;

    dx  = x1 - x0;  dy  = y1 - y0;
    adx = dx < 0 ? -dx : dx;
    ady = dy < 0 ? -dy : dy;
    sx  = dx < 0 ? -1 : 1;
    sy  = dy < 0 ? -1 : 1;

    set_pixel(x0, y0);
    if (adx >= ady)
    {
        err = adx >> 1;
        while (x0 != x1)
        {
            err -= ady;
            if (err < 0) { y0 += sy; err += adx; }
            x0 += sx;
            set_pixel(x0, y0);
        }
    }
    else
    {
        err = ady >> 1;
        while (y0 != y1)
        {
            err -= adx;
            if (err < 0) { x0 += sx; err += ady; }
            y0 += sy;
            set_pixel(x0, y0);
        }
    }
}

static void draw_circle(int cx, int cy, int r)
{
    int x = r, y = 0, err = 1 - r;
    while (x >= y)
    {
        set_pixel(cx + x, cy + y);
        set_pixel(cx - x, cy + y);
        set_pixel(cx + x, cy - y);
        set_pixel(cx - x, cy - y);
        set_pixel(cx + y, cy + x);
        set_pixel(cx - y, cy + x);
        set_pixel(cx + y, cy - x);
        set_pixel(cx - y, cy - x);
        y++;
        if (err < 0)
            err += 2 * y + 1;
        else
        {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

static void clear_fb(void)
{
    unsigned i;
    RIA.addr0 = back_buf;
    RIA.step0 = 1;
    for (i = 0; i < FB_SIZE / 8u; i++)
    {
        RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0;
        RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0;
    }
}

/* ---------- Main ---------- */

void main(void)
{
    int frame = 0;
    int f;
    uint8_t v;
    int e;

    /* Pre-compute all 180 frames using math coprocessor and measure elapsed time. */
    {
        clock_t t0, t1, elapsed;
        int ch;

        t0 = clock();
        for (f = 0; f < NUM_FRAMES; f++)
            precompute_frame(f);
        t1 = clock();

        elapsed = t1 - t0;                         /* ticks, CLOCKS_PER_SEC = 100 */
        printf("Precompute: %ld.%02ld s\n",
               (long)(elapsed / CLOCKS_PER_SEC),
               (long)((elapsed % CLOCKS_PER_SEC)));
    }

    /* Select 640x360 canvas */
    xreg_vga_canvas(4);

    back_buf = FB_A; clear_fb();
    back_buf = FB_B; clear_fb();

    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, x_wrap,           false);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, y_wrap,           false);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, x_pos_px,         0);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, y_pos_px,         0);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, width_px,         FB_WIDTH);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, height_px,        FB_HEIGHT);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_data_ptr,    FB_A);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(3, 0, CFG_ADDR, 0, 0, 0);

    back_buf = FB_B;

    while (1)
    {
        clear_fb();

        RIA.step0 = 0;
        if (frame < (NUM_FRAMES / 2))
        {
            /* First half-revolution: wireframe edges */
            for (e = 0; e < 12; e++)
                draw_line(pre_x[frame][edges[e][0]], pre_y[frame][edges[e][0]],
                          pre_x[frame][edges[e][1]], pre_y[frame][edges[e][1]]);
        }
        else
        {
            /* Second half-revolution: vertex circles */
            for (e = 0; e < 8; e++)
                // draw_circle(pre_x[frame][e], pre_y[frame][e], pre_r[frame][e]);
                set_pixel(pre_x[frame][e], pre_y[frame][e]);
        }

        v = RIA.vsync;
        while (RIA.vsync == v)
            ;

        xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_data_ptr, back_buf);
        back_buf = (back_buf == FB_A) ? FB_B : FB_A;

        frame++;
        if (frame >= NUM_FRAMES) frame = 0;
    }
}
