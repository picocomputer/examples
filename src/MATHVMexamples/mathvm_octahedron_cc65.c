/*
 * MATHVM octahedron animation.
 *
 * One MATHVM batch call per frame projects all 6 vertices of a regular
 * octahedron from XRAM input records to packed int16 x/y output records.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "mathvm_client.h"

#define KEYBOARD_ADDR 0xFFE0u

#define KEY_F4         0x3Du
#define KEY_LEFT_ALT   0xE2u
#define KEY_RIGHT_ALT  0xE6u

#define FB_WIDTH   640
#define FB_HEIGHT  360
#define FB_SIZE    28800u
#define FB_A       0x0000u
#define FB_B       0x7080u
#define CFG_ADDR   0xFF00u

#define XRAM_VERT_IN 0xE100u

#define OCTA_VERTS 6
#define OCTA_EDGES 12

static const mx_vec3i_t verts[OCTA_VERTS] = {
    {  0,  56,   0 },
    { 56,   0,   0 },
    {  0,   0,  56 },
    {-56,   0,   0 },
    {  0,   0, -56 },
    {  0, -56,   0 },
};

static const unsigned char edges[OCTA_EDGES][2] = {
    {0,1}, {0,2}, {0,3}, {0,4},
    {5,1}, {5,2}, {5,3}, {5,4},
    {1,2}, {2,3}, {3,4}, {4,1},
};

static const unsigned char bitmask[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static int proj_x[OCTA_VERTS];
static int proj_y[OCTA_VERTS];
static mx_point2i_t proj_points[OCTA_VERTS];
static unsigned back_buf;

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

static bool point_reasonable(int x, int y)
{
    return x > -FB_WIDTH && x < (FB_WIDTH * 2) &&
           y > -FB_HEIGHT && y < (FB_HEIGHT * 2);
}

static void draw_line(int x0, int y0, int x1, int y1)
{
    int dx;
    int dy;
    int adx;
    int ady;
    int sx;
    int sy;
    int err;

    dx  = x1 - x0;
    dy  = y1 - y0;
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
            if (err < 0)
            {
                y0 += sy;
                err += adx;
            }
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
            if (err < 0)
            {
                x0 += sx;
                err += ady;
            }
            y0 += sy;
            set_pixel(x0, y0);
        }
    }
}

static void clear_fb(void)
{
    unsigned i;

    RIA.addr0 = back_buf;
    RIA.step0 = 1;
    for (i = 0; i < FB_SIZE / 8u; ++i)
    {
        RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0;
        RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0; RIA.rw0 = 0;
    }
}

static bool project_vertices(int angle)
{
    mx_client_result_t call;
    uint8_t i;

    call = mx_client_project_vec3i_batch_yrot30(angle,
                                                200,
                                                320,
                                                180,
                                                XRAM_VERT_IN,
                                                verts,
                                                proj_points,
                                                OCTA_VERTS);
    if (call.status != MX_OK || call.out_words != 0u)
        return false;

    for (i = 0; i < OCTA_VERTS; ++i)
    {
        proj_x[i] = (int)proj_points[i].x;
        proj_y[i] = (int)proj_points[i].y;
    }
    return true;
}

void main(void)
{
    int angle = 0;
    uint8_t v;
    int e;

    xreg_vga_canvas(4);

    back_buf = FB_A;
    clear_fb();
    back_buf = FB_B;
    clear_fb();

    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, x_wrap,           false);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, y_wrap,           false);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, x_pos_px,         0);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, y_pos_px,         0);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, width_px,         FB_WIDTH);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, height_px,        FB_HEIGHT);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_data_ptr,    FB_A);
    xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_palette_ptr, 0xFFFF);

    xreg_vga_mode(3, 0, CFG_ADDR, 0, 0, 0);
    xreg_ria_keyboard(KEYBOARD_ADDR);

    back_buf = FB_B;

    while (1)
    {
        if (alt_f4_pressed())
            break;

        clear_fb();
        if (!project_vertices(angle))
            break;

        RIA.step0 = 0;
        for (e = 0; e < OCTA_VERTS; ++e)
            set_pixel(proj_x[e], proj_y[e]);

        if (angle < 180)
        {
            for (e = 0; e < OCTA_EDGES; ++e)
            {
                int ax = proj_x[edges[e][0]];
                int ay = proj_y[edges[e][0]];
                int bx = proj_x[edges[e][1]];
                int by = proj_y[edges[e][1]];

                if (point_reasonable(ax, ay) && point_reasonable(bx, by))
                    draw_line(ax, ay, bx, by);
            }
        }

        v = RIA.vsync;
        while (RIA.vsync == v)
            ;

        xram0_struct_set(CFG_ADDR, vga_mode3_config_t, xram_data_ptr, back_buf);
        back_buf = (back_buf == FB_A) ? FB_B : FB_A;

        angle += 2;
        if (angle >= 360)
            angle = 0;
    }

    xreg_ria_keyboard(0xFFFF);
    exit(0);
}
