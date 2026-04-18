/*
 * Standalone copy for examples.
 * Keep in sync with ../mathvm_client.c as needed by on-target callers.
 */

#include "mathvm_client.h"
#include <string.h>

#define RIA_XSTACK (*(volatile uint8_t *)0xFFEC)
#define RIA_RW0    (*(volatile uint8_t *)0xFFE4)
#define RIA_STEP0  (*(volatile int8_t *)0xFFE5)
#define RIA_ADDR0  (*(volatile uint16_t *)0xFFE6)
#define RIA_OP     (*(volatile uint8_t *)0xFFEF)

typedef unsigned int (*ria_wait_fn_t)(void);
static const ria_wait_fn_t ria_wait = (ria_wait_fn_t)0xFFF1;
static uint8_t mx_batch_frame[MX_MAX_FRAME];

static mx_client_result_t mx_client_error_result(uint8_t status)
{
    mx_client_result_t result;

    result.status = status;
    result.out_words = 0u;
    return result;
}

static uint8_t mx_u32_msb_index(uint32_t value)
{
    uint8_t index = 0u;

    while ((value >>= 1) != 0u)
        ++index;
    return index;
}

static uint32_t mx_u32_to_f32_bits_scaled(uint32_t magnitude, uint8_t frac_bits)
{
    uint8_t msb;
    int16_t exponent;
    uint32_t mantissa;

    if (magnitude == 0u)
        return 0u;

    msb = mx_u32_msb_index(magnitude);
    exponent = (int16_t)msb - (int16_t)frac_bits + 127;
    if (msb <= 23u)
        mantissa = magnitude << (23u - msb);
    else
        mantissa = magnitude >> (msb - 23u);

    return ((uint32_t)exponent << 23) | (mantissa & 0x007FFFFFu);
}

static mx_word_t mx_word_from_i32_scaled(int32_t value, uint8_t frac_bits)
{
    mx_word_t word;
    uint32_t magnitude;

    if (value < 0)
    {
        magnitude = (uint32_t)(-(value + 1)) + 1u;
        word.u32 = 0x80000000u | mx_u32_to_f32_bits_scaled(magnitude, frac_bits);
    }
    else
    {
        magnitude = (uint32_t)value;
        word.u32 = mx_u32_to_f32_bits_scaled(magnitude, frac_bits);
    }
    return word;
}

static void mx_xram_write_u32_seq(uint16_t addr, uint32_t value)
{
    RIA_ADDR0 = addr;
    RIA_STEP0 = 1;
    RIA_RW0 = (uint8_t)(value & 0xFFu);
    RIA_RW0 = (uint8_t)((value >> 8) & 0xFFu);
    RIA_RW0 = (uint8_t)((value >> 16) & 0xFFu);
    RIA_RW0 = (uint8_t)(value >> 24);
}

static uint32_t mx_xram_read_u32_seq(uint16_t addr)
{
    uint32_t value;

    RIA_ADDR0 = addr;
    RIA_STEP0 = 1;
    value  = (uint32_t)RIA_RW0;
    value |= (uint32_t)RIA_RW0 << 8;
    value |= (uint32_t)RIA_RW0 << 16;
    value |= (uint32_t)RIA_RW0 << 24;
    return value;
}

static void mx_append_u8(uint8_t *frame, uint16_t *len, uint8_t value)
{
    frame[(*len)++] = value;
}

static void mx_append_u16le(uint8_t *frame, uint16_t *len, uint16_t value)
{
    frame[(*len)++] = (uint8_t)(value & 0xFFu);
    frame[(*len)++] = (uint8_t)(value >> 8);
}

static void mx_append_u32le(uint8_t *frame, uint16_t *len, uint32_t value)
{
    frame[(*len)++] = (uint8_t)(value & 0xFFu);
    frame[(*len)++] = (uint8_t)((value >> 8) & 0xFFu);
    frame[(*len)++] = (uint8_t)((value >> 16) & 0xFFu);
    frame[(*len)++] = (uint8_t)(value >> 24);
}

static void mx_append_header(uint8_t *frame,
                             uint16_t *len,
                             uint8_t flags,
                             uint8_t prog_len,
                             uint8_t local_words,
                             uint8_t out_words,
                             uint8_t stack_words,
                             uint16_t xram_in,
                             uint16_t xram_out,
                             uint16_t count)
{
    mx_append_u8(frame, len, 0x4D);
    mx_append_u8(frame, len, 0x01);
    mx_append_u8(frame, len, flags);
    mx_append_u8(frame, len, MX_HEADER_BYTES);
    mx_append_u8(frame, len, prog_len);
    mx_append_u8(frame, len, local_words);
    mx_append_u8(frame, len, out_words);
    mx_append_u8(frame, len, stack_words);
    mx_append_u16le(frame, len, xram_in);
    mx_append_u16le(frame, len, xram_out);
    mx_append_u16le(frame, len, count);
    mx_append_u16le(frame, len, 0x0000u);
}

static void mx_append_words(uint8_t *frame,
                            uint16_t *len,
                            const mx_word_t *words,
                            uint8_t word_count)
{
    uint8_t i;

    for (i = 0; i < word_count; ++i)
        mx_append_u32le(frame, len, words[i].u32);
}

static void mx_push_frame_reverse(const uint8_t *frame, uint16_t frame_len)
{
    while (frame_len > 0)
        RIA_XSTACK = frame[--frame_len];
}

static void mx_drain_words(uint8_t word_count)
{
    while (word_count-- > 0)
    {
        (void)RIA_XSTACK;
        (void)RIA_XSTACK;
        (void)RIA_XSTACK;
        (void)RIA_XSTACK;
    }
}

static void mx_read_words(mx_word_t *out, uint8_t word_count)
{
    uint8_t i;
    uint8_t j;

    for (i = 0; i < word_count; ++i)
    {
        uint32_t value = 0;

        for (j = 0; j < 4; ++j)
            value |= ((uint32_t)RIA_XSTACK) << (j * 8);
        out[i].u32 = value;
    }
}

mx_client_result_t mx_client_call_frame(const uint8_t *frame,
                                        uint16_t frame_len,
                                        mx_word_t *out,
                                        uint8_t out_cap_words)
{
    mx_client_result_t result;
    unsigned int ax;

    mx_push_frame_reverse(frame, frame_len);

    RIA_OP = RIA_OP_MATHVM;
    ax = ria_wait();

    result.status = (uint8_t)(ax & 0x00FFu);
    result.out_words = (uint8_t)(ax >> 8);

    if (out == NULL || out_cap_words < result.out_words)
    {
        mx_drain_words(result.out_words);
        if (result.status == MX_OK && out_cap_words < result.out_words)
            result.status = MX_ERR_PROGRAM;
        return result;
    }

    mx_read_words(out, result.out_words);
    return result;
}

void mx_client_xram_write_vec3i_array(uint16_t xram_addr,
                                      const mx_vec3i_t *vecs,
                                      uint16_t count)
{
    uint16_t i;

    for (i = 0; i < count; ++i)
    {
        mx_xram_write_u32_seq(xram_addr, mx_word_from_i32_scaled(vecs[i].x, 0u).u32);
        xram_addr = (uint16_t)(xram_addr + 4u);
        mx_xram_write_u32_seq(xram_addr, mx_word_from_i32_scaled(vecs[i].y, 0u).u32);
        xram_addr = (uint16_t)(xram_addr + 4u);
        mx_xram_write_u32_seq(xram_addr, mx_word_from_i32_scaled(vecs[i].z, 0u).u32);
        xram_addr = (uint16_t)(xram_addr + 4u);
    }
}

void mx_client_xram_read_point2i_array(uint16_t xram_addr,
                                       mx_point2i_t *points,
                                       uint16_t count)
{
    uint16_t i;

    for (i = 0; i < count; ++i)
    {
        uint32_t raw = mx_xram_read_u32_seq(xram_addr);

        points[i].x = (int16_t)(raw & 0xFFFFu);
        points[i].y = (int16_t)(raw >> 16);
        xram_addr = (uint16_t)(xram_addr + 4u);
    }
}

mx_client_result_t mx_client_call_batch(const mx_client_batch_desc_t *desc,
                                        mx_word_t *out,
                                        uint8_t out_cap_words)
{
    uint16_t frame_len;

    if (desc == 0)
        return mx_client_error_result(MX_ERR_HEADER);
    if (desc->program == 0)
        return mx_client_error_result(MX_ERR_HEADER);
    if (desc->local_words != 0u && desc->locals == 0)
        return mx_client_error_result(MX_ERR_HEADER);

    if (desc->local_words > MX_MAX_LOCALS)
        return mx_client_error_result(MX_ERR_HEADER);
    if (desc->prog_len > MX_MAX_PROG)
        return mx_client_error_result(MX_ERR_HEADER);
    if (desc->out_words > MX_MAX_OUT)
        return mx_client_error_result(MX_ERR_HEADER);
    if (desc->stack_words > MX_MAX_STACK)
        return mx_client_error_result(MX_ERR_HEADER);

    frame_len = MX_HEADER_BYTES;
    frame_len = (uint16_t)(frame_len + (uint16_t)desc->local_words * MX_WORD_BYTES);
    frame_len = (uint16_t)(frame_len + desc->prog_len);
    if (frame_len > MX_MAX_FRAME)
        return mx_client_error_result(MX_ERR_HEADER);

    frame_len = 0u;
    mx_append_header(mx_batch_frame, &frame_len,
                     desc->flags,
                     desc->prog_len,
                     desc->local_words,
                     desc->out_words,
                     desc->stack_words,
                     desc->xram_in,
                     desc->xram_out,
                     desc->count);
    if (desc->local_words != 0u)
        mx_append_words(mx_batch_frame, &frame_len, desc->locals, desc->local_words);
    memcpy(&mx_batch_frame[frame_len], desc->program, desc->prog_len);
    frame_len = (uint16_t)(frame_len + desc->prog_len);

    return mx_client_call_frame(mx_batch_frame, frame_len, out, out_cap_words);
}

mx_client_result_t mx_client_m3v3l(const mx_word_t mat3[9],
                                   const mx_word_t vec3[3],
                                   mx_word_t out[3])
{
    uint8_t frame[16u + (12u * 4u) + 5u];
    uint16_t len = 0;

    mx_append_header(frame, &len, 0x00u, 5u, 12u, 3u, 8u, 0xFFFFu, 0xFFFFu, 0x0001u);
    mx_append_words(frame, &len, mat3, 9u);
    mx_append_words(frame, &len, vec3, 3u);
    mx_append_u8(frame, &len, MX_M3V3L);
    mx_append_u8(frame, &len, 0x00);
    mx_append_u8(frame, &len, 0x09);
    mx_append_u8(frame, &len, MX_RET);
    mx_append_u8(frame, &len, 0x03);

    return mx_client_call_frame(frame, len, out, 3u);
}

mx_client_result_t mx_client_spr2l_bbox(const mx_word_t affine2x3[6],
                                        const mx_word_t sprite[4],
                                        mx_word_t out[4])
{
    uint8_t frame[16u + (10u * 4u) + 6u];
    uint16_t len = 0;

    mx_append_header(frame, &len, 0x00u, 6u, 10u, 4u, 8u, 0xFFFFu, 0xFFFFu, 0x0001u);
    mx_append_words(frame, &len, affine2x3, 6u);
    mx_append_words(frame, &len, sprite, 4u);
    mx_append_u8(frame, &len, MX_SPR2L);
    mx_append_u8(frame, &len, 0x00);
    mx_append_u8(frame, &len, 0x06);
    mx_append_u8(frame, &len, 0x02);
    mx_append_u8(frame, &len, MX_RET);
    mx_append_u8(frame, &len, 0x04);

    return mx_client_call_frame(frame, len, out, 4u);
}

mx_client_result_t mx_client_spr2l_corners(const mx_word_t affine2x3[6],
                                           const mx_word_t sprite[4],
                                           mx_word_t out[8])
{
    uint8_t frame[16u + (10u * 4u) + 6u];
    uint16_t len = 0;

    mx_append_header(frame, &len, 0x00u, 6u, 10u, 8u, 8u, 0xFFFFu, 0xFFFFu, 0x0001u);
    mx_append_words(frame, &len, affine2x3, 6u);
    mx_append_words(frame, &len, sprite, 4u);
    mx_append_u8(frame, &len, MX_SPR2L);
    mx_append_u8(frame, &len, 0x00);
    mx_append_u8(frame, &len, 0x06);
    mx_append_u8(frame, &len, 0x01);
    mx_append_u8(frame, &len, MX_RET);
    mx_append_u8(frame, &len, 0x08);

    return mx_client_call_frame(frame, len, out, 8u);
}

mx_client_result_t mx_client_m3v3p2x(const mx_word_t mat3[9],
                                     const mx_word_t camera[3],
                                     uint16_t xram_in,
                                     uint16_t xram_out,
                                     uint16_t count)
{
    static const uint8_t program[] = {
        MX_M3V3P2X, 0x00, 0x09, MX_HALT
    };
    mx_word_t locals[12];
    mx_client_batch_desc_t desc;

    memcpy(&locals[0], mat3, 9u * sizeof(mx_word_t));
    memcpy(&locals[9], camera, 3u * sizeof(mx_word_t));

    desc.flags = MX_FLAG_USE_XRAM_IN | MX_FLAG_USE_XRAM_OUT;
    desc.xram_in = xram_in;
    desc.xram_out = xram_out;
    desc.count = count;
    desc.locals = locals;
    desc.local_words = 12u;
    desc.program = program;
    desc.prog_len = sizeof(program);
    desc.out_words = 0u;
    desc.stack_words = 12u;

    return mx_client_call_batch(&desc, NULL, 0u);
}

mx_client_result_t mx_client_m3v3p2x_q8_8(const int16_t mat3_q8_8[9],
                                          int16_t persp_d,
                                          int16_t screen_cx,
                                          int16_t screen_cy,
                                          uint16_t xram_in,
                                          uint16_t xram_out,
                                          uint16_t count)
{
    mx_word_t mat3_words[9];
    mx_word_t camera_words[3];
    uint8_t i;

    for (i = 0; i < 9u; ++i)
        mat3_words[i] = mx_word_from_i32_scaled(mat3_q8_8[i], 8u);
    camera_words[0] = mx_word_from_i32_scaled(persp_d, 0u);
    camera_words[1] = mx_word_from_i32_scaled(screen_cx, 0u);
    camera_words[2] = mx_word_from_i32_scaled(screen_cy, 0u);

    return mx_client_m3v3p2x(mat3_words, camera_words, xram_in, xram_out, count);
}

static int16_t mx_mul_q8_8(int16_t a, int16_t b)
{
    int32_t product = (int32_t)a * (int32_t)b;

    if (product >= 0)
        return (int16_t)((product + 128) >> 8);

    product = -product;
    return (int16_t)(-((product + 128) >> 8));
}

static int16_t mx_sin_deg_q8_8(int angle_deg)
{
    static const uint16_t sin_tab[91] = {
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

    while (angle_deg < 0)
        angle_deg += 360;
    while (angle_deg >= 360)
        angle_deg -= 360;

    if (angle_deg <= 90)
        return (int16_t)sin_tab[angle_deg];
    if (angle_deg <= 180)
        return (int16_t)sin_tab[180 - angle_deg];
    if (angle_deg <= 270)
        return (int16_t)(-(int16_t)sin_tab[angle_deg - 180]);
    return (int16_t)(-(int16_t)sin_tab[360 - angle_deg]);
}

static int16_t mx_cos_deg_q8_8(int angle_deg)
{
    return mx_sin_deg_q8_8(angle_deg + 90);
}

mx_client_result_t mx_client_m3v3p2x_yrot30(int angle_deg,
                                            int16_t persp_d,
                                            int16_t screen_cx,
                                            int16_t screen_cy,
                                            uint16_t xram_in,
                                            uint16_t xram_out,
                                            uint16_t count)
{
    int16_t mat3_q8_8[9];
    int16_t sin_a = mx_sin_deg_q8_8(angle_deg);
    int16_t cos_a = mx_cos_deg_q8_8(angle_deg);
    const int16_t sin30 = 128;
    const int16_t cos30 = 222;

    mat3_q8_8[0] = cos_a;
    mat3_q8_8[1] = 0;
    mat3_q8_8[2] = sin_a;
    mat3_q8_8[3] = mx_mul_q8_8(sin_a, sin30);
    mat3_q8_8[4] = cos30;
    mat3_q8_8[5] = (int16_t)-mx_mul_q8_8(cos_a, sin30);
    mat3_q8_8[6] = (int16_t)-mx_mul_q8_8(sin_a, cos30);
    mat3_q8_8[7] = sin30;
    mat3_q8_8[8] = mx_mul_q8_8(cos_a, cos30);

    return mx_client_m3v3p2x_q8_8(mat3_q8_8,
                                   persp_d,
                                   screen_cx,
                                   screen_cy,
                                   xram_in,
                                   xram_out,
                                   count);
}

mx_client_result_t mx_client_project_vec3i_batch_yrot30(int angle_deg,
                                                        int16_t persp_d,
                                                        int16_t screen_cx,
                                                        int16_t screen_cy,
                                                        uint16_t xram_base,
                                                        const mx_vec3i_t *vecs,
                                                        mx_point2i_t *points,
                                                        uint16_t count)
{
    mx_client_result_t result;
    uint32_t total_bytes;
    uint16_t xram_in;
    uint16_t xram_out;

    if ((count != 0u && vecs == 0) || (count != 0u && points == 0))
        return mx_client_error_result(MX_ERR_HEADER);

    total_bytes = (uint32_t)count * 16u;
    if ((uint32_t)xram_base + total_bytes > 0x10000u)
        return mx_client_error_result(MX_ERR_BAD_XRAM);

    xram_in = xram_base;
    xram_out = (uint16_t)(xram_base + (uint16_t)count * 12u);

    mx_client_xram_write_vec3i_array(xram_in, vecs, count);
    result = mx_client_m3v3p2x_yrot30(angle_deg,
                                      persp_d,
                                      screen_cx,
                                      screen_cy,
                                      xram_in,
                                      xram_out,
                                      count);
    if (result.status != MX_OK)
        return result;

    mx_client_xram_read_point2i_array(xram_out, points, count);
    return result;
}
