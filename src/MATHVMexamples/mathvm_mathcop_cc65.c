/*
 * MATHVM Math Coprocessor Test
 *
 * Based on rp6502_examples/src/MTHexamples/mathcop.c, but runs the checks
 * through OS $80 and MATHVM bytecode.
 *
 * This file no longer depends on the detached legacy mth backend.
 */

#include "mathvm_client.h"
#include <stdio.h>
#include <time.h>

#define F32_0_0          0x00000000UL
#define F32_1_0          0x3F800000UL
#define F32_2_0          0x40000000UL
#define F32_3_0          0x40400000UL
#define F32_3_5          0x40600000UL
#define F32_4_0          0x40800000UL
#define F32_5_0          0x40A00000UL
#define F32_5_5          0x40B00000UL
#define F32_7_0          0x40E00000UL
#define F32_8_0          0x41000000UL
#define F32_PI_OVER_180  0x3C8EFA35UL
#define F32_10000        0x461C4000UL
#define F32_NEG_2_0      0xC0000000UL

static unsigned int passed;
static unsigned int failed;

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

static mx_client_result_t run_unary_u32(uint8_t op, uint32_t arg, mx_word_t *out)
{
    uint8_t frame[32];
    uint16_t len = 0;

    append_header(frame, &len, 8u, 1u, 4u);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, arg);
    append_u8(frame, &len, op);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    return mx_client_call_frame(frame, len, out, 1u);
}

static mx_client_result_t run_binary_u32(uint8_t op, uint32_t a, uint32_t b, mx_word_t *out)
{
    uint8_t frame[40];
    uint16_t len = 0;

    append_header(frame, &len, 13u, 1u, 4u);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, a);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, b);
    append_u8(frame, &len, op);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    return mx_client_call_frame(frame, len, out, 1u);
}

static mx_client_result_t run_unary_f32(uint8_t op, uint32_t arg_bits, mx_word_t *out)
{
    uint8_t frame[32];
    uint16_t len = 0;

    append_header(frame, &len, 8u, 1u, 4u);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, arg_bits);
    append_u8(frame, &len, op);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    return mx_client_call_frame(frame, len, out, 1u);
}

static mx_client_result_t run_binary_f32(uint8_t op, uint32_t a_bits, uint32_t b_bits, mx_word_t *out)
{
    uint8_t frame[40];
    uint16_t len = 0;

    append_header(frame, &len, 13u, 1u, 4u);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, a_bits);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, b_bits);
    append_u8(frame, &len, op);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    return mx_client_call_frame(frame, len, out, 1u);
}

static mx_client_result_t run_binary_d64(uint8_t op,
                                         uint32_t alo,
                                         uint32_t ahi,
                                         uint32_t blo,
                                         uint32_t bhi,
                                         mx_word_t *out)
{
    uint8_t frame[64];
    uint16_t len = 0;

    append_header(frame, &len, 23u, 2u, 8u);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, alo);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, ahi);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, blo);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, bhi);
    append_u8(frame, &len, op);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x02);

    return mx_client_call_frame(frame, len, out, 2u);
}

static long mathvm_sin10000(int deg)
{
    uint8_t frame[64];
    uint16_t len = 0;
    mx_word_t out[1];
    mx_client_result_t call;

    append_header(frame, &len, 23u, 1u, 8u);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, (uint32_t)deg);
    append_u8(frame, &len, MX_ITOF);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_PI_OVER_180);
    append_u8(frame, &len, MX_FMUL);
    append_u8(frame, &len, MX_FSIN);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_10000);
    append_u8(frame, &len, MX_FMUL);
    append_u8(frame, &len, MX_FTOI);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    call = mx_client_call_frame(frame, len, out, 1u);
    if (call.status != 0 || call.out_words != 1)
        return 0;
    return (long)out[0].i32;
}

static long mathvm_cos10000(int deg)
{
    uint8_t frame[64];
    uint16_t len = 0;
    mx_word_t out[1];
    mx_client_result_t call;

    append_header(frame, &len, 23u, 1u, 8u);
    append_u8(frame, &len, MX_PUSHI);
    append_u32le(frame, &len, (uint32_t)deg);
    append_u8(frame, &len, MX_ITOF);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_PI_OVER_180);
    append_u8(frame, &len, MX_FMUL);
    append_u8(frame, &len, MX_FCOS);
    append_u8(frame, &len, MX_PUSHF);
    append_u32le(frame, &len, F32_10000);
    append_u8(frame, &len, MX_FMUL);
    append_u8(frame, &len, MX_FTOI);
    append_u8(frame, &len, MX_RET);
    append_u8(frame, &len, 0x01);

    call = mx_client_call_frame(frame, len, out, 1u);
    if (call.status != 0 || call.out_words != 1)
        return 0;
    return (long)out[0].i32;
}

static void check_int(const char *name, int result, int expected)
{
    if (result == expected)
    {
        printf("PASS: %s\n", name);
        ++passed;
    }
    else
    {
        printf("FAIL: %s  got=%d  exp=%d\n", name, result, expected);
        ++failed;
    }
}

static void check_long(const char *name, unsigned long result, unsigned long expected)
{
    if (result == expected)
    {
        printf("PASS: %s\n", name);
        ++passed;
    }
    else
    {
        printf("FAIL: %s  got=%08lX  exp=%08lX\n", name, result, expected);
        ++failed;
    }
}

static long bhaskara_sin10000(int deg)
{
    long p = (long)deg * (180 - deg);
    return 40000L * p / (40500L - p);
}

static long bhaskara_cos10000(int deg)
{
    return bhaskara_sin10000(90 - deg);
}

static void benchmark_sine(void)
{
    clock_t t_start;
    clock_t t_vm;
    clock_t t_cpu;
    unsigned long ratio_tenths;
    int i;
    long dummy = 0;

    puts("\n-- Benchmark: Sine Table (91 values, 0-90 deg) --");

    t_start = clock();
    for (i = 0; i <= 90; ++i)
        dummy += mathvm_sin10000(i);
    t_vm = clock() - t_start;

    t_start = clock();
    for (i = 0; i <= 90; ++i)
        dummy += bhaskara_sin10000(i);
    t_cpu = clock() - t_start;

    printf("MATHVM       : %lu ms\n",
           (unsigned long)t_vm * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    printf("CPU Bhaskara : %lu ms\n",
           (unsigned long)t_cpu * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    if (t_vm > 0)
    {
        ratio_tenths = ((unsigned long)t_cpu * 10UL + (unsigned long)t_vm / 2UL) /
                       (unsigned long)t_vm;
        printf("CPU/MATHVM ratio: %lu.%lux\n", ratio_tenths / 10UL, ratio_tenths % 10UL);
    }
    (void)dummy;
}

static void benchmark_cosine(void)
{
    clock_t t_start;
    clock_t t_vm;
    clock_t t_cpu;
    unsigned long ratio_tenths;
    int i;
    long dummy = 0;

    puts("\n-- Benchmark: Cosine Table (91 values, 0-90 deg) --");

    t_start = clock();
    for (i = 0; i <= 90; ++i)
        dummy += mathvm_cos10000(i);
    t_vm = clock() - t_start;

    t_start = clock();
    for (i = 0; i <= 90; ++i)
        dummy += bhaskara_cos10000(i);
    t_cpu = clock() - t_start;

    printf("MATHVM       : %lu ms\n",
           (unsigned long)t_vm * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    printf("CPU Bhaskara : %lu ms\n",
           (unsigned long)t_cpu * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    if (t_vm > 0)
    {
        ratio_tenths = ((unsigned long)t_cpu * 10UL + (unsigned long)t_vm / 2UL) /
                       (unsigned long)t_vm;
        printf("CPU/MATHVM ratio: %lu.%lux\n", ratio_tenths / 10UL, ratio_tenths % 10UL);
    }
    (void)dummy;
}

int main(void)
{
    mx_word_t out[2];
    mx_client_result_t call;

    puts("MATHVM Math Coprocessor Test");
    puts("===========================");
    passed = 0;
    failed = 0;

    puts("\n-- Integer --");

    call = run_binary_u32(MX_MUL8U, 3u, 7u, out);
    check_int("MUL8   3*7=21", (call.status == 0) ? (int)out[0].u32 : -1, 21);

    call = run_binary_u32(MX_MUL8U, 255u, 255u, out);
    check_int("MUL8   255*255=65025", (call.status == 0) ? (int)out[0].u32 : -1, (int)65025u);

    call = run_binary_u32(MX_MUL16U, 200u, 300u, out);
    check_long("MUL16  200*300=60000", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 60000UL);

    call = run_binary_u32(MX_MUL16U, 1000u, 1000u, out);
    check_long("MUL16  1000*1000=1000000", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 1000000UL);

    call = run_binary_u32(MX_MUL16S, (uint32_t)(int32_t)-5, (uint32_t)(int32_t)-3, out);
    check_long("MULS16 -5*-3=15", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 15UL);

    call = run_binary_u32(MX_MUL16S, 7u, (uint32_t)(int32_t)-4, out);
    check_long("MULS16 7*-4=-28", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, (unsigned long)-28L);

    call = run_binary_u32(MX_DIV16U, 100u, 7u, out);
    check_long("DIV16  100/7=14r2", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 0x0002000EUL);

    call = run_binary_u32(MX_DIV16U, 65535u, 256u, out);
    check_long("DIV16  65535/256=255r255", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 0x00FF00FFUL);

    call = run_unary_u32(MX_SQRT32U, 144u, out);
    check_int("SQRT32 sqrt(144)=12", (call.status == 0) ? (int)out[0].u32 : -1, 12);

    call = run_unary_u32(MX_SQRT32U, 10000u, out);
    check_int("SQRT32 sqrt(10000)=100", (call.status == 0) ? (int)out[0].u32 : -1, 100);

    puts("\n-- Float32 --");

    call = run_binary_f32(MX_FADD, F32_2_0, F32_3_5, out);
    check_long("FADD   2.0+3.5=5.5", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_5_5);

    call = run_binary_f32(MX_FSUB, F32_7_0, F32_3_5, out);
    check_long("FSUB   7.0-3.5=3.5", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_3_5);

    call = run_binary_f32(MX_FMUL, F32_2_0, F32_3_5, out);
    check_long("FMUL   2.0*3.5=7.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_7_0);

    call = run_binary_f32(MX_FDIV, F32_7_0, F32_2_0, out);
    check_long("FDIV   7.0/2.0=3.5", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_3_5);

    call = run_unary_f32(MX_FSQRT, F32_4_0, out);
    check_long("FSQRT  sqrt(4.0)=2.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_2_0);

    call = run_unary_f32(MX_FSIN, F32_0_0, out);
    check_long("FSIN   sin(0.0)=0.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_0_0);

    call = run_unary_f32(MX_FCOS, F32_0_0, out);
    check_long("FCOS   cos(0.0)=1.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_1_0);

    call = run_binary_f32(MX_FATAN2, F32_0_0, F32_1_0, out);
    check_long("FATAN2 atan2(0,1)=0.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_0_0);

    call = run_binary_f32(MX_FPOW, F32_2_0, F32_3_0, out);
    check_long("FPOW   pow(2,3)=8.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_8_0);

    call = run_unary_f32(MX_FLOG, F32_1_0, out);
    check_long("FLOG   log(1.0)=0.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_0_0);

    call = run_unary_f32(MX_FEXP, F32_0_0, out);
    check_long("FEXP   exp(0.0)=1.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_1_0);

    call = run_unary_f32(MX_FTOI, F32_3_0, out);
    check_long("FTOI   (int)3.0=3", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 3UL);

    call = run_unary_f32(MX_FTOI, F32_NEG_2_0, out);
    check_long("FTOI   (int)-2.0=-2", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, (unsigned long)-2L);

    call = run_unary_u32(MX_ITOF, 5u, out);
    check_long("ITOF   (float)5=5.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_5_0);

    call = run_unary_u32(MX_ITOF, 0u, out);
    check_long("ITOF   (float)0=0.0", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, F32_0_0);

    puts("\n-- Double64 --");

    call = run_binary_d64(MX_DADD, 0x00000000UL, 0x3FF00000UL, 0x00000000UL, 0x3FF00000UL, out);
    check_long("DADD   1.0+1.0=2.0 lo", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 0x00000000UL);
    check_long("DADD   1.0+1.0=2.0 hi", (call.status == 0) ? out[1].u32 : 0xFFFFFFFFUL, 0x40000000UL);

    call = run_binary_d64(MX_DMUL, 0x00000000UL, 0x3FF80000UL, 0x00000000UL, 0x40000000UL, out);
    check_long("DMUL   2.0*1.5=3.0 lo", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 0x00000000UL);
    check_long("DMUL   2.0*1.5=3.0 hi", (call.status == 0) ? out[1].u32 : 0xFFFFFFFFUL, 0x40080000UL);

    call = run_binary_d64(MX_DDIV, 0x00000000UL, 0x40080000UL, 0x00000000UL, 0x3FF80000UL, out);
    check_long("DDIV   3.0/1.5=2.0 lo", (call.status == 0) ? out[0].u32 : 0xFFFFFFFFUL, 0x00000000UL);
    check_long("DDIV   3.0/1.5=2.0 hi", (call.status == 0) ? out[1].u32 : 0xFFFFFFFFUL, 0x40000000UL);

    benchmark_sine();
    benchmark_cosine();

    puts("");
    printf("Results: %u passed, %u failed\n", passed, failed);
    return 0;
}
