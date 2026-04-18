/*
 * Minimal cc65 negative-test example for Picocomputer 6502 MATHVM.
 *
 * Executes five invalid v1 frames through OS $80 and prints returned status:
 *   - bad magic
 *   - bad header
 *   - unsupported flag
 *   - stack underflow
 *   - bad local
 */

#include "mathvm_client.h"
#include <stdio.h>

#define MX_ERR_MAGIC       0x01
#define MX_ERR_HEADER      0x03
#define MX_ERR_STACK_UDF   0x07
#define MX_ERR_BAD_LOCAL   0x08
#define MX_ERR_UNSUPPORTED 0x0B

#define MX_RET   0x02
#define MX_LDS   0x12
#define MX_DROP  0x19

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
                          uint8_t magic,
                          uint8_t version,
                          uint8_t flags,
                          uint8_t hdr_size,
                          uint8_t prog_len,
                          uint8_t local_words,
                          uint8_t out_words,
                          uint8_t stack_words)
{
    append_u8(frame, len, magic);
    append_u8(frame, len, version);
    append_u8(frame, len, flags);
    append_u8(frame, len, hdr_size);
    append_u8(frame, len, prog_len);
    append_u8(frame, len, local_words);
    append_u8(frame, len, out_words);
    append_u8(frame, len, stack_words);
    append_u16le(frame, len, 0xFFFFu);
    append_u16le(frame, len, 0xFFFFu);
    append_u16le(frame, len, 0x0001u);
    append_u16le(frame, len, 0x0000u);
}

static uint16_t build_bad_magic_frame(uint8_t *frame)
{
    uint16_t len = 0;

    append_header(frame, &len, 0x00, 0x01, 0x00, 0x10, 0x02, 0x00, 0x00, 0x01);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x00);
    return len;
}

static uint16_t build_bad_header_frame(uint8_t *frame)
{
    uint16_t len = 0;

    append_header(frame, &len, 0x4D, 0x01, 0x00, 0x0F, 0x02, 0x00, 0x00, 0x01);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x00);
    return len;
}

static uint16_t build_unsupported_flag_frame(uint8_t *frame)
{
    uint16_t len = 0;

    append_header(frame, &len, 0x4D, 0x01, 0x10, 0x10, 0x02, 0x00, 0x00, 0x01);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x00);
    return len;
}

static uint16_t build_stack_underflow_frame(uint8_t *frame)
{
    uint16_t len = 0;

    append_header(frame, &len, 0x4D, 0x01, 0x00, 0x10, 0x01, 0x00, 0x00, 0x01);
    append_u8(frame, &len, MX_DROP);
    return len;
}

static uint16_t build_bad_local_frame(uint8_t *frame)
{
    uint16_t len = 0;

    append_header(frame, &len, 0x4D, 0x01, 0x00, 0x10, 0x02, 0x01, 0x01, 0x02);
    append_u32le(frame, &len, 0x42F60000UL);
    append_u8(frame, &len, MX_LDS);
    append_u8(frame, &len, 0x01);
    return len;
}

static void run_case(const char *name,
                     uint16_t (*build_frame)(uint8_t *),
                     uint8_t expected_status)
{
    uint8_t frame[32];
    uint16_t frame_len;
    mx_client_result_t call;

    frame_len = build_frame(frame);
    call = mx_client_call_frame(frame, frame_len, NULL, 0);

    printf("%s: status=%u words=%u expected=%u\n",
           name,
           call.status,
           call.out_words,
           expected_status);
}

int main(void)
{
    puts("MATHVM negative tests for OS $80");
    run_case("bad magic", build_bad_magic_frame, MX_ERR_MAGIC);
    run_case("bad header", build_bad_header_frame, MX_ERR_HEADER);
    run_case("unsupported flag", build_unsupported_flag_frame, MX_ERR_UNSUPPORTED);
    run_case("stack underflow", build_stack_underflow_frame, MX_ERR_STACK_UDF);
    run_case("bad local", build_bad_local_frame, MX_ERR_BAD_LOCAL);
    return 0;
}
