/*
 * MATHVM batch dodecahedron debug program.
 *
 * Uploads the same 20 dodecahedron vertices to XRAM, runs the batch
 * projection for a few angles, and prints the resulting x/y pairs.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mathvm_client.h"

#define XRAM_VERT_IN   0xE100u
#define XRAM_VERT_OUT  0xE200u

#define DODECA_VERTS 20

static const mx_vec3i_t verts[DODECA_VERTS] = {
    {-20, -20, -20},
    { 20, -20, -20},
    { 20,  20, -20},
    {-20,  20, -20},
    {-20, -20,  20},
    { 20, -20,  20},
    { 20,  20,  20},
    {-20,  20,  20},
    {  0, -12, -32},
    {  0,  12, -32},
    {  0, -12,  32},
    {  0,  12,  32},
    {-12, -32,   0},
    { 12, -32,   0},
    {-12,  32,   0},
    { 12,  32,   0},
    {-32,   0, -12},
    { 32,   0, -12},
    {-32,   0,  12},
    { 32,   0,  12},
};

static int proj_x[DODECA_VERTS];
static int proj_y[DODECA_VERTS];

static uint32_t xram_read_u32_seq(uint16_t addr)
{
    uint32_t value;

    RIA.addr0 = addr;
    RIA.step0 = 1;
    value  = (uint32_t)RIA.rw0;
    value |= (uint32_t)RIA.rw0 << 8;
    value |= (uint32_t)RIA.rw0 << 16;
    value |= (uint32_t)RIA.rw0 << 24;
    return value;
}

static void read_projected_vertices(void)
{
    uint16_t addr = XRAM_VERT_OUT;
    uint8_t i;

    for (i = 0; i < DODECA_VERTS; ++i)
    {
        uint32_t raw = xram_read_u32_seq(addr);

        proj_x[i] = (int)(int16_t)(raw & 0xFFFFu);
        proj_y[i] = (int)(int16_t)(raw >> 16);
        addr += 4u;
    }
}

static mx_client_result_t project_vertices(int angle)
{
    mx_client_result_t call;
    call = mx_client_m3v3p2x_yrot30(angle,
                                    200,
                                    320,
                                    180,
                                    XRAM_VERT_IN,
                                    XRAM_VERT_OUT,
                                    DODECA_VERTS);
    if (call.status == MX_OK)
        read_projected_vertices();
    return call;
}

static void dump_angle(int angle)
{
    mx_client_result_t call;
    int xmin = 32767;
    int ymin = 32767;
    int xmax = -32768;
    int ymax = -32768;
    uint8_t i;

    call = project_vertices(angle);
    printf("\nangle=%d status=%u words=%u\n", angle, call.status, call.out_words);
    if (call.status != MX_OK)
        return;

    for (i = 0; i < DODECA_VERTS; ++i)
    {
        if (proj_x[i] < xmin) xmin = proj_x[i];
        if (proj_x[i] > xmax) xmax = proj_x[i];
        if (proj_y[i] < ymin) ymin = proj_y[i];
        if (proj_y[i] > ymax) ymax = proj_y[i];
    }

    printf("range x=%d..%d y=%d..%d\n", xmin, xmax, ymin, ymax);
    for (i = 0; i < 8u; ++i)
        printf("v%u: %d,%d\n", i, proj_x[i], proj_y[i]);

    puts("raw out bytes:");
    for (i = 0; i < 4u; ++i)
    {
        uint32_t raw = xram_read_u32_seq((uint16_t)(XRAM_VERT_OUT + (uint16_t)i * 4u));
        printf("o%u: %08lX\n", i, (unsigned long)raw);
    }
}

void main(void)
{
    puts("MATHVM dodecahedron batch debug");
    mx_client_xram_write_vec3i_array(XRAM_VERT_IN, verts, DODECA_VERTS);
    puts("raw in words:");
    printf("in0.x=%08lX in0.y=%08lX in0.z=%08lX\n",
           (unsigned long)xram_read_u32_seq(XRAM_VERT_IN + 0u),
           (unsigned long)xram_read_u32_seq(XRAM_VERT_IN + 4u),
           (unsigned long)xram_read_u32_seq(XRAM_VERT_IN + 8u));
    printf("in1.x=%08lX in1.y=%08lX in1.z=%08lX\n",
           (unsigned long)xram_read_u32_seq(XRAM_VERT_IN + 12u),
           (unsigned long)xram_read_u32_seq(XRAM_VERT_IN + 16u),
           (unsigned long)xram_read_u32_seq(XRAM_VERT_IN + 20u));
    dump_angle(0);
    dump_angle(90);
    dump_angle(180);
}
