/*
 * 3D Cube Animation
 *
 * Rotating wireframe cube (60x60x60 px) viewed from 30 degrees elevation.
 * Uses VGA mode 3 (640x360, 1bpp), double buffering, and RP2350 math coprocessor.
 *
 * XRAM layout:
 *   0x0000 - 0x707F  Framebuffer A (28 800 bytes)
 *   0x7080 - 0xE0FF  Framebuffer B (28 800 bytes)
 *   0xFF00 - 0xFF0D  vga_mode3_config_t
 *
 * Each frame:
 *   1. Clear back buffer
 *   2. Project 8 vertices with Ry(angle) * Rx(30 deg) using math coprocessor
 *   3. Draw 12 edges with Bresenham line algorithm
 *   4. Wait vsync, flip displayed buffer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdint.h>
#include <stdbool.h>

/* ---------- VGA framebuffer layout ---------- */

/* 640x360, 1bpp: 640/8 = 80 bytes/row, 360 rows = 28 800 bytes per buffer */
#define FB_WIDTH   640
#define FB_HEIGHT  360
#define FB_SIZE    28800u      /* bytes per framebuffer */
#define FB_A       0x0000u     /* framebuffer A base address in XRAM */
#define FB_B       0x7080u     /* framebuffer B base address (= FB_A + FB_SIZE) */
#define CFG_ADDR   0xFF00u     /* vga_mode3_config_t location in XRAM */

/* Screen center for orthographic projection */
#define CX  320
#define CY  180

/* ---------- Math coprocessor constants (IEEE 754 fp32_t) ---------- */

#define F32_PI_OVER_180  0x3C8EFA35UL  /* pi/180  ~= 0.017453 */
#define F32_SIN30        0x3F000000UL  /* sin(30 deg) = 0.5    */
#define F32_COS30        0x3F5DB3D7UL  /* cos(30 deg) ~= 0.866 */
#define F32_D            0x43480000UL  /* viewer distance = 200.0 */

/* Base circle radius in pixels — edit this value to change circle size */
#define BASE_CIRCLE_R    2

/* ---------- Cube geometry ---------- */

/* Half-size of the cube: edges are 60 px long (= 2 * HALF) */
#define HALF 50

/* 8 vertices of the cube + 1 face-center marker, stored as signed bytes */
static const signed char verts[9][3] = {
    {-HALF, -HALF, -HALF}, /* 0 back-bottom-left  */
    {+HALF, -HALF, -HALF}, /* 1 back-bottom-right */
    {+HALF, +HALF, -HALF}, /* 2 back-top-right    */
    {-HALF, +HALF, -HALF}, /* 3 back-top-left     */
    {-HALF, -HALF, +HALF}, /* 4 front-bottom-left */
    {+HALF, -HALF, +HALF}, /* 5 front-bottom-right*/
    {+HALF, +HALF, +HALF}, /* 6 front-top-right   */
    {-HALF, +HALF, +HALF}, /* 7 front-top-left    */
    {    0,     0, +HALF}, /* 8 front face center  */
};

/* 12 edges: each entry is a pair of vertex indices */
static const unsigned char edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},  /* back face   */
    {4,5}, {5,6}, {6,7}, {7,4},  /* front face  */
    {0,4}, {1,5}, {2,6}, {3,7},  /* side edges  */
};

/* Bit mask table: bit to set for pixel column within byte (MSB = leftmost) */
static const unsigned char bitmask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

/* Projected 2D screen coordinates and perspective radius (8 vertices + face center) */
static int proj_x[9];
static int proj_y[9];
static int proj_r[9];

/* XRAM base address of the buffer being rendered (back buffer) */
static unsigned back_buf;

/* ---------- Pixel and line drawing ---------- */

/* set_pixel: draw one white pixel at (x, y) in back_buf.
 * Caller must set RIA.step0 = 0 before calling.
 * Uses unsigned cast to handle negative coordinates efficiently. */
static void set_pixel(int x, int y)
{
    unsigned addr;
    if ((unsigned)x >= (unsigned)FB_WIDTH || (unsigned)y >= (unsigned)FB_HEIGHT)
        return;
    /* row offset = y * 80 = y*64 + y*16 (no software multiply needed) */
    addr = back_buf
         + (((unsigned)y << 6) + ((unsigned)y << 4))
         + ((unsigned)x >> 3);
    RIA.addr0 = addr;
    RIA.rw0 = RIA.rw0 | bitmask[x & 7];  /* read-modify-write */
}

/* Bresenham's line algorithm */
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

/* Bresenham midpoint circle algorithm */
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
        {
            err += 2 * y + 1;
        }
        else
        {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

/* ---------- Framebuffer clear ---------- */

/* Fill back_buf with zeros (black).
 * Uses 8x unrolled loop for speed; leaves RIA.step0 = 1. */
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

/* ---------- 3D vertex projection ---------- */

/* Project all 8 cube vertices onto 2D screen.
 * Transform: Ry(angle) then Rx(+30 deg), perspective projection.
 *   Ry(a): x' =  x*cos(a) + z*sin(a)
 *           z' = -x*sin(a) + z*cos(a)
 *   Rx(30): sy = y*cos30 - z'*sin30   (screen Y)
 *           sz = y*sin30 + z'*cos30   (depth toward viewer)
 * Perspective divide: w = D / (D - sz),  viewer at -D on the Z axis
 * Screen:  sx = x' * w,  sy = sy * w
 */
static void project_vertices(int angle)
{
    fp32_t fa, sin_a, cos_a, f_base_r;
    fp32_t fx, fy, fz, rx, ry, rz, sy, sz, w;
    int i;

    f_base_r = mth_itof(BASE_CIRCLE_R);

    fa    = mth_mulf(mth_itof(angle), F32_PI_OVER_180);
    sin_a = mth_sinf(fa);
    cos_a = mth_cosf(fa);

    for (i = 0; i < 9; i++)
    {
        fx = mth_itof((int)verts[i][0]);
        fy = mth_itof((int)verts[i][1]);
        fz = mth_itof((int)verts[i][2]);

        /* Ry(angle) */
        rx = mth_addf(mth_mulf(fx, cos_a), mth_mulf(fz, sin_a));
        ry = fy;
        rz = mth_subf(mth_mulf(fz, cos_a), mth_mulf(fx, sin_a));

        /* Rx(+30 deg): tilt for 30-degree elevation view */
        sy = mth_subf(mth_mulf(ry, F32_COS30), mth_mulf(rz, F32_SIN30));
        sz = mth_addf(mth_mulf(ry, F32_SIN30), mth_mulf(rz, F32_COS30));

        /* Perspective: w = D / (D - sz);  near (sz>0) → larger, far (sz<0) → smaller */
        w = mth_divf(F32_D, mth_subf(F32_D, sz));

        proj_x[i] = CX + (int)mth_ftoi(mth_mulf(rx, w));
        proj_y[i] = CY - (int)mth_ftoi(mth_mulf(sy, w));
        proj_r[i] = (int)mth_ftoi(mth_mulf(f_base_r, w));
        if (proj_r[i] < 1) proj_r[i] = 1;
    }
}

/* ---------- Main ---------- */

void main(void)
{
    int angle = 0;
    uint8_t v;
    int e;

    /* Select 640x360 canvas (vga_640_360 = 4) */
    xreg_vga_canvas(4);

    /* Zero both framebuffers before activating mode */
    back_buf = FB_A; clear_fb();
    back_buf = FB_B; clear_fb();

    /* Write mode3 config struct into XRAM */
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, x_wrap,           false);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, y_wrap,           false);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, x_pos_px,         0);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, y_pos_px,         0);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, width_px,         FB_WIDTH);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, height_px,        FB_HEIGHT);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_data_ptr,    FB_A);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    /* Activate mode 3, 1bpp; scanline_end=0 defaults to canvas height (360) */
    xreg_vga_mode(3, 0, CFG_ADDR, 0, 0, 0);

    /* Display FB_A (blank), render into FB_B */
    back_buf = FB_B;

    while (1)
    {
        /* Render frame into back buffer */
        clear_fb();
        project_vertices(angle);

        RIA.step0 = 0;  /* required for read-modify-write in set_pixel */
        if (angle < 180)
        {
            for (e = 0; e < 12; e++)
                draw_line(proj_x[edges[e][0]], proj_y[edges[e][0]],
                          proj_x[edges[e][1]], proj_y[edges[e][1]]);
            // draw_circle(proj_x[8], proj_y[8], proj_r[8]); /* front face center */
        }
        else
        {
            for (e = 0; e < 8; e++)
                draw_circle(proj_x[e], proj_y[e], proj_r[e]);
            // draw_circle(proj_x[8], proj_y[8], proj_r[8]); /* front face center */
        }

        /* Wait for next vsync */
        v = RIA.vsync;
        while (RIA.vsync == v)
            ;

        /* Flip: point config at newly rendered buffer */
        xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_data_ptr, back_buf);

        /* Swap: old front becomes new back */
        back_buf = (back_buf == FB_A) ? FB_B : FB_A;

        /* Advance rotation: 2 degrees per frame (~6 s per full revolution at 30 fps) */
        angle += 2;
        if (angle >= 360) angle = 0;
    }
}
