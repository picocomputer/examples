/*
 * MATHVM batch benchmark.
 *
 * Compares:
 *   1. Pure CPU integer approximation
 *   2. One MATHVM call with an internal bytecode loop
 *
 * The benchmark computes:
 *   sum_{deg=0..90} sin(deg*pi/180) * 10000
 *   sum_{deg=0..90} cos(deg*pi/180) * 10000
 *
 * Both paths return the final float32 checksum as raw IEEE 754 bits.
 */

#include "mathvm_client.h"
#include <stdio.h>
#include <time.h>

#define RIA_XSTACK   (*(volatile uint8_t *)0xFFEC)
#define RIA_OP       (*(volatile uint8_t *)0xFFEF)
#define RIA_A        (*(volatile uint8_t *)0xFFF4)
#define RIA_XREG     (*(volatile uint8_t *)0xFFF6)
#define RIA_SREG_LO  (*(volatile uint8_t *)0xFFF8)
#define RIA_SREG_HI  (*(volatile uint8_t *)0xFFF9)

/*
 * Legacy scalar mth opcodes are intentionally left commented out.
 * The old mth backend is no longer linked into firmware, so this benchmark
 * now compares pure CPU code against a single MATHVM call.
 *
#define MTH_FADD     0x38
#define MTH_FMUL     0x3A
#define MTH_FSIN     0x3D
#define MTH_FCOS     0x3E
#define MTH_ITOF     0x44
 */

#define F32_0_0          0x00000000UL
#define F32_1_0          0x3F800000UL
#define F32_PI_OVER_180  0x3C8EFA35UL
#define F32_10000        0x461C4000UL
#define F32_91_0         0x42B60000UL

typedef unsigned int (*ria_wait_fn_t)(void);
static const ria_wait_fn_t ria_wait = (ria_wait_fn_t)0xFFF1;

static void append_u8(uint8_t *buf, uint8_t *len, uint8_t value)
{
    buf[(*len)++] = value;
}

static void append_u16le(uint8_t *buf, uint8_t *len, uint16_t value)
{
    buf[(*len)++] = (uint8_t)(value & 0xFFu);
    buf[(*len)++] = (uint8_t)(value >> 8);
}

static void append_u32le(uint8_t *buf, uint8_t *len, uint32_t value)
{
    buf[(*len)++] = (uint8_t)(value & 0xFFu);
    buf[(*len)++] = (uint8_t)((value >> 8) & 0xFFu);
    buf[(*len)++] = (uint8_t)((value >> 16) & 0xFFu);
    buf[(*len)++] = (uint8_t)(value >> 24);
}

static void push_u32(uint32_t value)
{
    RIA_XSTACK = (uint8_t)(value >> 24);
    RIA_XSTACK = (uint8_t)((value >> 16) & 0xFFu);
    RIA_XSTACK = (uint8_t)((value >> 8) & 0xFFu);
    RIA_XSTACK = (uint8_t)(value & 0xFFu);
}

static void set_axsreg_u32(uint32_t value)
{
    RIA_A = (uint8_t)(value & 0xFFu);
    RIA_XREG = (uint8_t)((value >> 8) & 0xFFu);
    RIA_SREG_LO = (uint8_t)((value >> 16) & 0xFFu);
    RIA_SREG_HI = (uint8_t)(value >> 24);
}

static uint32_t call_u32(uint8_t op)
{
    unsigned int ax;

    RIA_OP = op;
    ax = ria_wait();
    return ((uint32_t)ax & 0xFFFFu) |
           ((uint32_t)RIA_SREG_LO << 16) |
           ((uint32_t)RIA_SREG_HI << 24);
}

/*
static uint32_t scalar_itof(int value)
{
    set_axsreg_u32((uint32_t)value);
    return call_u32(MTH_ITOF);
}

static uint32_t scalar_fsin(uint32_t value)
{
    set_axsreg_u32(value);
    return call_u32(MTH_FSIN);
}

static uint32_t scalar_fcos(uint32_t value)
{
    set_axsreg_u32(value);
    return call_u32(MTH_FCOS);
}

static uint32_t scalar_fmul(uint32_t a, uint32_t b)
{
    push_u32(a);
    set_axsreg_u32(b);
    return call_u32(MTH_FMUL);
}

static uint32_t scalar_fadd(uint32_t a, uint32_t b)
{
    push_u32(a);
    set_axsreg_u32(b);
    return call_u32(MTH_FADD);
}

static uint32_t scalar_sum_trig10000(uint8_t trig_op)
{
    uint32_t sum = F32_0_0;
    int deg;

    for (deg = 0; deg <= 90; ++deg)
    {
        uint32_t angle = scalar_itof(deg);
        uint32_t radians = scalar_fmul(angle, F32_PI_OVER_180);
        uint32_t trig = (trig_op == MX_FSIN) ? scalar_fsin(radians) : scalar_fcos(radians);
        uint32_t scaled = scalar_fmul(trig, F32_10000);
        sum = scalar_fadd(sum, scaled);
    }

    return sum;
}
*/

static long bhaskara_sin10000(int deg)
{
    long p = (long)deg * (180 - deg);
    return 40000L * p / (40500L - p);
}

static long bhaskara_cos10000(int deg)
{
    return bhaskara_sin10000(90 - deg);
}

static long cpu_sum_trig10000(uint8_t trig_op)
{
    long sum = 0;
    int deg;

    for (deg = 0; deg <= 90; ++deg)
        sum += (trig_op == MX_FSIN) ? bhaskara_sin10000(deg) : bhaskara_cos10000(deg);

    return sum;
}

static uint8_t build_sum_trig_frame(uint8_t trig_op, uint8_t *frame)
{
    uint8_t prog[80];
    uint8_t prog_len = 0;
    uint8_t frame_len = 0;
    uint8_t loop_start;
    uint8_t rel_pos;
    int8_t rel;

    loop_start = prog_len;

    append_u8(prog, &prog_len, MX_LDS);  append_u8(prog, &prog_len, 0x01);
    append_u8(prog, &prog_len, MX_LDS);  append_u8(prog, &prog_len, 0x00);
    append_u8(prog, &prog_len, MX_PUSHF); append_u32le(prog, &prog_len, F32_PI_OVER_180);
    append_u8(prog, &prog_len, MX_FMUL);
    append_u8(prog, &prog_len, trig_op);
    append_u8(prog, &prog_len, MX_PUSHF); append_u32le(prog, &prog_len, F32_10000);
    append_u8(prog, &prog_len, MX_FMUL);
    append_u8(prog, &prog_len, MX_FADD);
    append_u8(prog, &prog_len, MX_STS);  append_u8(prog, &prog_len, 0x01);

    append_u8(prog, &prog_len, MX_LDS);  append_u8(prog, &prog_len, 0x00);
    append_u8(prog, &prog_len, MX_PUSHF); append_u32le(prog, &prog_len, F32_1_0);
    append_u8(prog, &prog_len, MX_FADD);
    append_u8(prog, &prog_len, MX_STS);  append_u8(prog, &prog_len, 0x00);

    append_u8(prog, &prog_len, MX_LDS);  append_u8(prog, &prog_len, 0x02);
    append_u8(prog, &prog_len, MX_PUSHF); append_u32le(prog, &prog_len, F32_1_0);
    append_u8(prog, &prog_len, MX_FSUB);
    append_u8(prog, &prog_len, MX_DUP);
    append_u8(prog, &prog_len, MX_STS);  append_u8(prog, &prog_len, 0x02);
    append_u8(prog, &prog_len, MX_JNZ);
    rel_pos = prog_len;
    append_u8(prog, &prog_len, 0x00);

    append_u8(prog, &prog_len, MX_LDS);  append_u8(prog, &prog_len, 0x01);
    append_u8(prog, &prog_len, MX_RET);  append_u8(prog, &prog_len, 0x01);

    rel = (int8_t)((int)loop_start - (int)(rel_pos + 1u));
    prog[rel_pos] = (uint8_t)rel;

    append_u8(frame, &frame_len, 0x4D);
    append_u8(frame, &frame_len, 0x01);
    append_u8(frame, &frame_len, 0x00);
    append_u8(frame, &frame_len, MX_HEADER_BYTES);
    append_u8(frame, &frame_len, prog_len);
    append_u8(frame, &frame_len, 0x03);
    append_u8(frame, &frame_len, 0x01);
    append_u8(frame, &frame_len, 0x10);
    append_u16le(frame, &frame_len, 0xFFFFu);
    append_u16le(frame, &frame_len, 0xFFFFu);
    append_u16le(frame, &frame_len, 0x0001u);
    append_u16le(frame, &frame_len, 0x0000u);

    append_u32le(frame, &frame_len, F32_0_0);
    append_u32le(frame, &frame_len, F32_0_0);
    append_u32le(frame, &frame_len, F32_91_0);

    {
        uint8_t i;
        for (i = 0; i < prog_len; ++i)
            append_u8(frame, &frame_len, prog[i]);
    }

    return frame_len;
}

static uint32_t mathvm_sum_trig10000(uint8_t trig_op)
{
    uint8_t frame[128];
    uint8_t frame_len;
    mx_word_t out[1];
    mx_client_result_t call;

    frame_len = build_sum_trig_frame(trig_op, frame);
    call = mx_client_call_frame(frame, frame_len, out, 1u);
    if (call.status != 0 || call.out_words != 1)
        return 0xFFFFFFFFUL;
    return out[0].u32;
}

static void benchmark_one(const char *name, uint8_t trig_op)
{
    clock_t t0;
    clock_t t_cpu;
    clock_t t_mathvm;
    long cpu_sum;
    uint32_t mathvm_sum;

    puts("");
    printf("-- Benchmark: %s sum(0..90) --\n", name);

    t0 = clock();
    cpu_sum = cpu_sum_trig10000(trig_op);
    t_cpu = clock() - t0;

    t0 = clock();
    mathvm_sum = mathvm_sum_trig10000(trig_op);
    t_mathvm = clock() - t0;

    printf("pure CPU      : %lu ms\n",
           (unsigned long)t_cpu * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    printf("one MATHVM call: %lu ms\n",
           (unsigned long)t_mathvm * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    printf("CPU sum (x10000): %ld\n", cpu_sum);
    printf("MATHVM sum bits: %08lX\n", (unsigned long)mathvm_sum);

    if (t_mathvm > 0)
        printf("CPU/MATHVM ratio   : %lux\n", (unsigned long)(t_cpu / t_mathvm));
}

int main(void)
{
    puts("MATHVM Batch Benchmark");
    puts("=====================");

    benchmark_one("sine", MX_FSIN);
    benchmark_one("cosine", MX_FCOS);

    return 0;
}
