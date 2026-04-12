/*
 * 3D Cube Animation — PRE-COMPUTED variant
 *
 * All vertex screen coordinates for all 180 animation frames
 * (angle = 0, 2, 4, ... 358 degrees) are computed once at startup
 * and stored in RAM tables.  The animation loop only reads from
 * those tables — no math at all per frame.
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
 * Fixed-point convention: trig values scaled by 256.
 *   sin_tab[i] = round(sin(i degrees) * 256)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* Keyboard */

#define KEYBOARD_ADDR 0xFFE0u  /* keyboard bitmap location in XRAM */

/* USB HID keycodes used by the RIA keyboard bitmap */
#define KEY_F4         0x3Du
#define KEY_LEFT_ALT   0xE2u
#define KEY_RIGHT_ALT  0xE6u

static bool key_down(unsigned char code)
{
    RIA.addr1 = KEYBOARD_ADDR + (unsigned)(code >> 3);
    RIA.step1 = 0;
    return (RIA.rw1 & (1u << (code & 7))) != 0;
}

static bool alt_f4_pressed(void)
{
    return key_down(KEY_F4)
        && (key_down(KEY_LEFT_ALT) || key_down(KEY_RIGHT_ALT));
}

/* ---------- VGA framebuffer layout ---------- */

#define FB_WIDTH   640
#define FB_HEIGHT  360
#define FB_SIZE    28800u
#define FB_A       0x0000u
#define FB_B       0x7080u
#define CFG_ADDR   0xFF00u

#define CX  320
#define CY  180

/* ---------- Fixed-point trig constants (scale = 256) ---------- */

#define SIN30_FP  128
#define COS30_FP  222
#define PERSP_D   200

#define BASE_CIRCLE_R  2

/* ---------- Animation parameters ---------- */

#define ANGLE_STEP   2              /* degrees per frame */
#define NUM_FRAMES   (360 / ANGLE_STEP)   /* 180 */

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

/* ---------- Sine lookup table ---------- */

static const int sin_tab[91] = {
      0,   4,   9,  13,  18,  22,  27,  31,  36,  40,
     44,  49,  53,  58,  62,  66,  71,  75,  79,  83,
     88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
    128, 132, 136, 139, 143, 147, 150, 154, 158, 161,
    165, 168, 171, 175, 178, 181, 184, 187, 190, 193,
    196, 199, 202, 204, 207, 210, 212, 215, 217, 219,
    222, 224, 226, 228, 230, 232, 234, 236, 237, 239,
    241, 242, 243, 245, 246, 247, 248, 249, 250, 251,
    252, 253, 254, 254, 255, 255, 255, 256, 256, 256,
    256
};

static int sin_fp(int deg)
{
    if (deg <= 90)  return  sin_tab[deg];
    if (deg <= 180) return  sin_tab[180 - deg];
    if (deg <= 270) return -sin_tab[deg - 180];
    return                 -sin_tab[360 - deg];
}

static int cos_fp(int deg)
{
    deg += 90;
    if (deg >= 360) deg -= 360;
    return sin_fp(deg);
}

/* ---------- Pre-computed coordinate tables ---------- */

/*
 * 180 frames, 8 vertices each.
 * Total: 3 * 180 * 8 * 2 = 8 640 bytes.
 */
static int pre_x[NUM_FRAMES][8];
static int pre_y[NUM_FRAMES][8];
static int pre_r[NUM_FRAMES][8];

/* Compute screen coordinates for one frame and store in the tables. */
static void precompute_frame(int frame)
{
    int angle = frame * ANGLE_STEP;
    int sin_a = sin_fp(angle);
    int cos_a = cos_fp(angle);
    int x, y, z, rx, rz, sy, sz, denom;
    int i;

    for (i = 0; i < 8; i++)
    {
        x = (int)verts[i][0];
        y = (int)verts[i][1];
        z = (int)verts[i][2];

        rx = (x * cos_a + z * sin_a) >> 8;
        rz = (z * cos_a - x * sin_a) >> 8;

        sy = (y * COS30_FP - rz * SIN30_FP) >> 8;
        sz = (y * SIN30_FP + rz * COS30_FP) >> 8;

        denom = PERSP_D - sz;
        if (denom < 1) denom = 1;

        pre_x[frame][i] = CX + rx * PERSP_D / denom;
        pre_y[frame][i] = CY - sy * PERSP_D / denom;
        pre_r[frame][i] = BASE_CIRCLE_R * PERSP_D / denom;
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

    /* Pre-compute all 180 frames and measure elapsed time. */
    {
        clock_t t0, t1, elapsed;

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

    draw_circle(320,180,85);

    xreg_ria_keyboard(KEYBOARD_ADDR);

    back_buf = FB_B;

    while (1)
    {

        if (alt_f4_pressed()) break;
        
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
    xreg_ria_keyboard(0xFFFF);
    exit(0);
}
