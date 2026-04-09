/*
 * 3D Cube Animation
 *
 * Rotating wireframe cube viewed from 30 degrees elevation.
 * Uses VGA mode 3 (640x360, 1bpp), double buffering.
 * All 3D math done in 16-bit integer fixed-point on the 65C02.
 *
 * XRAM layout:
 *   0x0000 - 0x707F  Framebuffer A (28 800 bytes)
 *   0x7080 - 0xE0FF  Framebuffer B (28 800 bytes)
 *   0xFF00 - 0xFF0D  vga_mode3_config_t
 *
 * Fixed-point convention: trig values scaled by 256.
 *   sin_tab[i] = round(sin(i degrees) * 256)
 *   All intermediate products stay within signed 16-bit range.
 *
 * Each frame:
 *   1. Clear back buffer
 *   2. Project 9 points (8 vertices + front-face center) with integer math
 *   3. Draw 12 edges or 8 vertex circles (alternating half-revolutions)
 *      Front-face-center circle always drawn
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

/* Screen center */
#define CX  320
#define CY  180

/* ---------- Fixed-point trig constants (scale = 256) ---------- */

#define SIN30_FP  128   /* sin(30 deg) * 256 = 128  (exact: 0.5 * 256) */
#define COS30_FP  222   /* cos(30 deg) * 256 = 222  (0.8660254 * 256 = 221.7) */
#define PERSP_D   200   /* viewer distance in world units */

/* Base circle radius in pixels — edit this value to change circle size */
#define BASE_CIRCLE_R  2

/* ---------- Cube geometry ---------- */

/* Half-size of the cube */
#define HALF 50

/* 8 vertices of the cube + 1 face-center marker, stored as signed bytes */
static const signed char verts[8][3] = {
    {-HALF, -HALF, -HALF}, /* 0 back-bottom-left  */
    {+HALF, -HALF, -HALF}, /* 1 back-bottom-right */
    {+HALF, +HALF, -HALF}, /* 2 back-top-right    */
    {-HALF, +HALF, -HALF}, /* 3 back-top-left     */
    {-HALF, -HALF, +HALF}, /* 4 front-bottom-left */
    {+HALF, -HALF, +HALF}, /* 5 front-bottom-right*/
    {+HALF, +HALF, +HALF}, /* 6 front-top-right   */
    {-HALF, +HALF, +HALF}, /* 7 front-top-left    */
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

/* ---------- Sine lookup table: sin_tab[i] = round(sin(i deg) * 256), i = 0..90 ---------- */

static const int sin_tab[91] = {
      0,   4,   9,  13,  18,  22,  27,  31,  36,  40,  /* 0-9   */
     44,  49,  53,  58,  62,  66,  71,  75,  79,  83,  /* 10-19 */
     88,  92,  96, 100, 104, 108, 112, 116, 120, 124,  /* 20-29 */
    128, 132, 136, 139, 143, 147, 150, 154, 158, 161,  /* 30-39 */
    165, 168, 171, 175, 178, 181, 184, 187, 190, 193,  /* 40-49 */
    196, 199, 202, 204, 207, 210, 212, 215, 217, 219,  /* 50-59 */
    222, 224, 226, 228, 230, 232, 234, 236, 237, 239,  /* 60-69 */
    241, 242, 243, 245, 246, 247, 248, 249, 250, 251,  /* 70-79 */
    252, 253, 254, 254, 255, 255, 255, 256, 256, 256,  /* 80-89 */
    256                                                 /* 90    */
};

/* Returns sin(deg) * 256; deg must be 0..359 */
static int sin_fp(int deg)
{
    if (deg <= 90)  return  sin_tab[deg];
    if (deg <= 180) return  sin_tab[180 - deg];
    if (deg <= 270) return -sin_tab[deg - 180];
    return                 -sin_tab[360 - deg];
}

/* Returns cos(deg) * 256; deg must be 0..359 */
static int cos_fp(int deg)
{
    deg += 90;
    if (deg >= 360) deg -= 360;
    return sin_fp(deg);
}

/* ---------- Projected 2D screen coordinates and perspective radius ---------- */

static int proj_x[8];
static int proj_y[8];
static int proj_r[8];

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

/* ---------- 3D vertex projection (integer fixed-point, scale = 256) ---------- */

/* Project all 8 points onto 2D screen.
 * Transform: Ry(angle) then Rx(+30 deg), perspective projection.
 *
 *   Ry(a):  rx = (x*cos_a + z*sin_a) >> 8     (world units)
 *           rz = (z*cos_a - x*sin_a) >> 8
 *
 *   Rx(30): sy = (y*COS30 - rz*SIN30) >> 8    (screen Y, world units)
 *           sz = (y*SIN30 + rz*COS30) >> 8    (depth, world units)
 *
 * Perspective: w = PERSP_D / (PERSP_D - sz)
 *   screen_x = CX + rx * PERSP_D / (PERSP_D - sz)
 *   screen_y = CY - sy * PERSP_D / (PERSP_D - sz)
 *
 * All intermediate values stay within signed 16-bit range:
 *   x*cos_a: max 50*256 = 12800; sum: max 25600 < 32767
 *   sy/sz scaled: max ~21940 < 32767
 *   rx*PERSP_D: max 70*200 = 14000 < 32767
 */
static void project_vertices(int angle)
{
    int sin_a, cos_a;
    int x, y, z, rx, rz, sy, sz, denom;
    int i;

    sin_a = sin_fp(angle);
    cos_a = cos_fp(angle);

    for (i = 0; i < 8; i++)
    {
        x = (int)verts[i][0];
        y = (int)verts[i][1];
        z = (int)verts[i][2];

        /* Ry(angle): rotate in XZ plane */
        rx = (x * cos_a + z * sin_a) >> 8;
        rz = (z * cos_a - x * sin_a) >> 8;

        /* Rx(+30 deg): tilt for 30-degree elevation view */
        sy = (y * COS30_FP - rz * SIN30_FP) >> 8;
        sz = (y * SIN30_FP + rz * COS30_FP) >> 8;

        /* Perspective divide: near objects (sz > 0) appear larger */
        denom = PERSP_D - sz;
        if (denom < 1) denom = 1;

        proj_x[i] = CX + rx * PERSP_D / denom;
        proj_y[i] = CY - sy * PERSP_D / denom;
        proj_r[i] = BASE_CIRCLE_R * PERSP_D / denom;
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
        }
        else
        {
            for (e = 0; e < 8; e++)
                // draw_circle(proj_x[e], proj_y[e], proj_r[e]);
                set_pixel(proj_x[e], proj_y[e]);
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
