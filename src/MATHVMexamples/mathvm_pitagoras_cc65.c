/*
 * Minimal cc65 example for Picocomputer 6502 MATHVM.
 *
 * Computes the hypotenuse of a right triangle with legs 3 and 4:
 *   sqrt(3*3 + 4*4) = 5
 *
 * Expected output word as float32:
 *   5.0f
 */

#include "mathvm_client.h"
#include <stdio.h>

#define F32_3_0 0x40400000UL
#define F32_4_0 0x40800000UL

static void append_u8(uint8_t *frame, uint16_t *len, uint8_t value)
{
    frame[(*len)++] = value;
}

static void append_u16le(uint8_t *frame, uint16_t *len, uint16_t value)
{
    frame[(*len)++] = (uint8_t)(value & 0xFFu);
    frame[(*len)++] = (uint8_t)(value >> 8);
}

static void append_u32le(uint8_t *frame, uint16_t *len, uint32_t value)
{
    frame[(*len)++] = (uint8_t)(value & 0xFFu);
    frame[(*len)++] = (uint8_t)((value >> 8) & 0xFFu);
    frame[(*len)++] = (uint8_t)((value >> 16) & 0xFFu);
    frame[(*len)++] = (uint8_t)(value >> 24);
}

static void append_header(uint8_t *frame,
                          uint16_t *len,
                          uint8_t prog_len,
                          uint8_t out_words,
                          uint8_t stack_words)
{
    append_u8(frame, len, 0x4D);
    append_u8(frame, len, 0x01);
    append_u8(frame, len, 0x00);
    append_u8(frame, len, MX_HEADER_BYTES);
    append_u8(frame, len, prog_len);
    append_u8(frame, len, 0x00);
    append_u8(frame, len, out_words);
    append_u8(frame, len, stack_words);
    append_u16le(frame, len, 0xFFFFu);
    append_u16le(frame, len, 0xFFFFu);
    append_u16le(frame, len, 0x0001u);
    append_u16le(frame, len, 0x0000u);
}

int main(void)
{
    uint8_t frame[48];
    uint16_t len = 0;
    mx_client_result_t call;
    mx_word_t out[1];

    append_header(frame, &len, 26u, 1u, 4u);

    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_3_0);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_3_0);
    append_u8(frame, &len, MX_FMUL);

    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_4_0);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_4_0);
    append_u8(frame, &len, MX_FMUL);

    append_u8(frame, &len, MX_FADD);
    append_u8(frame, &len, MX_FSQRT);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    call = mx_client_call_frame(frame, len, out, 1u);

    printf("MATHVM status=%u words=%u\n", call.status, call.out_words);

    if (call.status == MX_OK && call.out_words == 1u)
    {
        printf("hypotenuse bits=%08lx\n", (unsigned long)out[0].u32);
        puts("expected: 40A00000");
    }

    return 0;
}
