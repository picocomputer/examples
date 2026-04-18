/*
 * Math Coprocessor Test
 *
 * Tests all operations of the RP6502 math coprocessor (mth module).
 * The coprocessor uses RP2350 Cortex-M33 hardware FPU exposed via RIA API.
 *
 * Since cc65 does not support float/double natively, float values are
 * represented as raw IEEE 754 bit patterns stored in unsigned long.
 *
 * Calling convention:
 *   - Last argument: fastcall register (ria_set_a / ria_set_ax / ria_set_axsreg)
 *   - Earlier arguments: xstack (ria_push_char / ria_push_int / ria_push_long)
 *   - 16-bit result: ria_call_int(op)
 *   - 32-bit result: ria_call_long(op)
 *   - Double result: result on xstack, read via two ria_pop_long() calls
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-License-Identifier: Unlicense
 */

#include <rp6502.h>
#include <stdio.h>
#include <time.h>

/* Math coprocessor opcodes */
#define MTH_MUL8    0x30  /* u8 * u8 -> u16 */
#define MTH_MUL16   0x31  /* u16 * u16 -> u32 */
#define MTH_MULS16  0x32  /* s16 * s16 -> s32 */
#define MTH_DIV16   0x33  /* u32 / u16 -> quot(u16) | rem(u16)<<16 */
#define MTH_SQRT32  0x34  /* sqrt(u32) -> u16 */
#define MTH_FADD    0x38  /* f32 + f32 -> f32 */
#define MTH_FSUB    0x39  /* f32 - f32 -> f32 */
#define MTH_FMUL    0x3A  /* f32 * f32 -> f32 */
#define MTH_FDIV    0x3B  /* f32 / f32 -> f32 */
#define MTH_FSQRT   0x3C  /* sqrt(f32) -> f32 */
#define MTH_FSIN    0x3D  /* sin(f32) -> f32 */
#define MTH_FCOS    0x3E  /* cos(f32) -> f32 */
#define MTH_FATAN2  0x3F  /* atan2(y,x) -> f32 */
#define MTH_FPOW    0x40  /* pow(base,exp) -> f32 */
#define MTH_FLOG    0x41  /* log(f32) -> f32  (natural log) */
#define MTH_FEXP    0x42  /* exp(f32) -> f32 */
#define MTH_FTOI    0x43  /* (s32)f32 */
#define MTH_ITOF    0x44  /* (f32)s32 */
#define MTH_DADD    0x48  /* f64 + f64 -> f64  (via xstack) */
#define MTH_DMUL    0x49  /* f64 * f64 -> f64  (via xstack) */
#define MTH_DDIV    0x4A  /* f64 / f64 -> f64  (via xstack) */

/* IEEE 754 float32 bit patterns (exact values used in tests) */
#define F32_0_0     0x00000000UL  /* 0.0  */
#define F32_1_0     0x3F800000UL  /* 1.0  */
#define F32_2_0     0x40000000UL  /* 2.0  */
#define F32_3_0     0x40400000UL  /* 3.0  */
#define F32_3_5     0x40600000UL  /* 3.5  */
#define F32_4_0     0x40800000UL  /* 4.0  */
#define F32_5_0     0x40A00000UL  /* 5.0  */
#define F32_5_5     0x40B00000UL  /* 5.5  */
#define F32_7_0     0x40E00000UL  /* 7.0  */
#define F32_8_0     0x41000000UL  /* 8.0  */
#define F32_PI_OVER_180 0x3C8EFA35UL  /* pi/180 ~= 0.017453292 */
#define F32_10000       0x461C4000UL  /* 10000.0 */

/* Push a double to xstack from two 32-bit halves.
 * hi32 is pushed first so that lo32 ends up at the top (lower xstack address).
 * This ensures firmware api_pop_n() reconstructs the correct little-endian double.
 */
#define push_double(lo32, hi32) \
    do { ria_push_long(hi32); ria_push_long(lo32); } while (0)

static unsigned int passed;
static unsigned int failed;

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

static void check_long(const char *name,
                       unsigned long result,
                       unsigned long expected)
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

/* Bhaskara I approximation: sin(d°) ~= 4d(180-d) / (40500 - d(180-d))
 * Exact at 0, 30, 90 deg. Max error ~0.1%. Pure 32-bit integer arithmetic.
 * Returns sin * 10000 (same scale as the coprocessor benchmark).
 */
static long bhaskara_sin10000(int deg)
{
    long p = (long)deg * (180 - deg);
    return 40000L * p / (40500L - p);
}

/* Bhaskara I approximation: cos(d°) = sin(90° - d°)
 * Uses bhaskara_sin10000 via the complementary angle identity.
 * Valid for 0 <= deg <= 90. Returns cos * 10000.
 */
static long bhaskara_cos10000(int deg)
{
    return bhaskara_sin10000(90 - deg);
}

static void benchmark_sine(void)
{
    clock_t t_start, t_cop, t_cpu;
    int i;
    long dummy = 0;

    puts("\n-- Benchmark: Sine Table (91 values, 0-90 deg) --");

    /* Coprocessor: ITOF + FMUL(deg->rad) + FSIN + FMUL(*10000) + FTOI */
    t_start = clock();
    for (i = 0; i <= 90; i++)
        dummy += mth_ftoi(mth_mulf(
            mth_sinf(mth_mulf(mth_itof(i), F32_PI_OVER_180)),
            F32_10000));
    t_cop = clock() - t_start;

    /* 65C02: Bhaskara I formula, no coprocessor, no lookup table */
    t_start = clock();
    for (i = 0; i <= 90; i++)
        dummy += bhaskara_sin10000(i);
    t_cpu = clock() - t_start;

    printf("Coprocessor : %lu ms\n",
           (unsigned long)t_cop * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    printf("CPU Bhaskara: %lu ms\n",
           (unsigned long)t_cpu * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    if (t_cop > 0)
        printf("CPU/cop ratio: %lux\n", (unsigned long)(t_cpu / t_cop));
    (void)dummy;
}

static void benchmark_cosine(void)
{
    clock_t t_start, t_cop, t_cpu;
    int i;
    long dummy = 0;

    puts("\n-- Benchmark: Cosine Table (91 values, 0-90 deg) --");

    /* Coprocessor: ITOF + FMUL(deg->rad) + FCOS + FMUL(*10000) + FTOI */
    t_start = clock();
    for (i = 0; i <= 90; i++)
        dummy += mth_ftoi(mth_mulf(
            mth_cosf(mth_mulf(mth_itof(i), F32_PI_OVER_180)),
            F32_10000));
    t_cop = clock() - t_start;

    /* 65C02: Bhaskara I formula, no coprocessor, no lookup table */
    t_start = clock();
    for (i = 0; i <= 90; i++)
        dummy += bhaskara_cos10000(i);
    t_cpu = clock() - t_start;

    printf("Coprocessor : %lu ms\n",
           (unsigned long)t_cop * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    printf("CPU Bhaskara: %lu ms\n",
           (unsigned long)t_cpu * 1000UL / (unsigned long)CLOCKS_PER_SEC);
    if (t_cop > 0)
        printf("CPU/cop ratio: %lux\n", (unsigned long)(t_cpu / t_cop));
    (void)dummy;
}

static void sine_table(void)
{
    int i;
    long sin_scaled;

    puts("\n-- Sine Table (0-90 degrees) --");

    for (i = 0; i <= 90; i++)
    {
        sin_scaled = mth_ftoi(mth_mulf(
            mth_sinf(mth_mulf(mth_itof(i), F32_PI_OVER_180)),
            F32_10000));

        printf("sin(%2d) = %d.%04d\n",
               i,
               (int)(sin_scaled / 10000L),
               (int)(sin_scaled % 10000L));
    }
}

static void cosine_table(void)
{
    int i;
    long cos_scaled;

    puts("\n-- Cosine Table (0-90 degrees) --");

    for (i = 0; i <= 90; i++)
    {
        cos_scaled = mth_ftoi(mth_mulf(
            mth_cosf(mth_mulf(mth_itof(i), F32_PI_OVER_180)),
            F32_10000));

        printf("cos(%2d) = %d.%04d\n",
               i,
               (int)(cos_scaled / 10000L),
               (int)(cos_scaled % 10000L));
    }
}

void main(void)
{
    unsigned long lo, hi;

    puts("Math Coprocessor Test");
    puts("=====================");
    passed = 0;
    failed = 0;

    /* ==================== INTEGER ==================== */

    puts("\n-- Integer --");

    /* MUL8: 3 * 7 = 21
     * a=3 on xstack (1 byte), b=7 fastcall (A)
     */
    ria_push_char(3);
    ria_set_a(7);
    check_int("MUL8   3*7=21",
              ria_call_int(MTH_MUL8), 21);

    /* MUL8: 255 * 255 = 65025  (max u8 inputs)
     * Tests that result fits in u16.
     */
    ria_push_char(255);
    ria_set_a(255);
    check_int("MUL8   255*255=65025",
              ria_call_int(MTH_MUL8), (int)65025u);

    /* MUL16: 200 * 300 = 60000
     * a=200 on xstack (2 bytes), b=300 fastcall (AX)
     */
    ria_push_int(200);
    ria_set_ax(300);
    check_long("MUL16  200*300=60000",
               (unsigned long)ria_call_long(MTH_MUL16), 60000UL);

    /* MUL16: 1000 * 1000 = 1000000  (tests u32 result) */
    ria_push_int(1000);
    ria_set_ax(1000);
    check_long("MUL16  1000*1000=1000000",
               (unsigned long)ria_call_long(MTH_MUL16), 1000000UL);

    /* MULS16: (-5) * (-3) = 15  (both negative) */
    ria_push_int(-5);
    ria_set_ax((unsigned int)-3);
    check_long("MULS16 -5*-3=15",
               (unsigned long)ria_call_long(MTH_MULS16), 15UL);

    /* MULS16: 7 * (-4) = -28  (negative result) */
    ria_push_int(7);
    ria_set_ax((unsigned int)-4);
    check_long("MULS16 7*-4=-28",
               (unsigned long)ria_call_long(MTH_MULS16), (unsigned long)-28L);

    /* DIV16: 100 / 7 = quotient 14, remainder 2
     * dividend=100 on xstack (4 bytes), divisor=7 fastcall (AX)
     * Result packed: low 16 = quotient, high 16 = remainder
     */
    ria_push_long(100UL);
    ria_set_ax(7);
    check_long("DIV16  100/7=14r2",
               (unsigned long)ria_call_long(MTH_DIV16), 0x0002000EUL);

    /* DIV16: 65535 / 256 = quotient 255, remainder 255 */
    ria_push_long(65535UL);
    ria_set_ax(256);
    check_long("DIV16  65535/256=255r255",
               (unsigned long)ria_call_long(MTH_DIV16), 0x00FF00FFUL);

    /* SQRT32: sqrt(144) = 12 */
    ria_set_axsreg(144UL);
    check_int("SQRT32 sqrt(144)=12",
              ria_call_int(MTH_SQRT32), 12);

    /* SQRT32: sqrt(10000) = 100 */
    ria_set_axsreg(10000UL);
    check_int("SQRT32 sqrt(10000)=100",
              ria_call_int(MTH_SQRT32), 100);

    /* ==================== FLOAT32 ==================== */

    puts("\n-- Float32 --");

    /* FADD: 2.0 + 3.5 = 5.5 = 0x40B00000 */
    ria_push_long(F32_2_0);
    ria_set_axsreg(F32_3_5);
    check_long("FADD   2.0+3.5=5.5",
               (unsigned long)ria_call_long(MTH_FADD), F32_5_5);

    /* FSUB: 7.0 - 3.5 = 3.5 */
    ria_push_long(F32_7_0);
    ria_set_axsreg(F32_3_5);
    check_long("FSUB   7.0-3.5=3.5",
               (unsigned long)ria_call_long(MTH_FSUB), F32_3_5);

    /* FMUL: 2.0 * 3.5 = 7.0 */
    ria_push_long(F32_2_0);
    ria_set_axsreg(F32_3_5);
    check_long("FMUL   2.0*3.5=7.0",
               (unsigned long)ria_call_long(MTH_FMUL), F32_7_0);

    /* FDIV: 7.0 / 2.0 = 3.5 */
    ria_push_long(F32_7_0);
    ria_set_axsreg(F32_2_0);
    check_long("FDIV   7.0/2.0=3.5",
               (unsigned long)ria_call_long(MTH_FDIV), F32_3_5);

    /* FSQRT: sqrt(4.0) = 2.0 */
    ria_set_axsreg(F32_4_0);
    check_long("FSQRT  sqrt(4.0)=2.0",
               (unsigned long)ria_call_long(MTH_FSQRT), F32_2_0);

    /* FSIN: sin(0.0) = 0.0  (exact) */
    ria_set_axsreg(F32_0_0);
    check_long("FSIN   sin(0.0)=0.0",
               (unsigned long)ria_call_long(MTH_FSIN), F32_0_0);

    /* FCOS: cos(0.0) = 1.0  (exact) */
    ria_set_axsreg(F32_0_0);
    check_long("FCOS   cos(0.0)=1.0",
               (unsigned long)ria_call_long(MTH_FCOS), F32_1_0);

    /* FATAN2: atan2(0.0, 1.0) = 0.0  (exact)
     * a=y=0.0 on xstack, b=x=1.0 fastcall
     */
    ria_push_long(F32_0_0);
    ria_set_axsreg(F32_1_0);
    check_long("FATAN2 atan2(0,1)=0.0",
               (unsigned long)ria_call_long(MTH_FATAN2), F32_0_0);

    /* FPOW: pow(2.0, 3.0) = 8.0  (exact)
     * a=base=2.0 on xstack, b=exp=3.0 fastcall
     */
    ria_push_long(F32_2_0);
    ria_set_axsreg(F32_3_0);
    check_long("FPOW   pow(2,3)=8.0",
               (unsigned long)ria_call_long(MTH_FPOW), F32_8_0);

    /* FLOG: log(1.0) = 0.0  (ln(1) = 0, exact) */
    ria_set_axsreg(F32_1_0);
    check_long("FLOG   log(1.0)=0.0",
               (unsigned long)ria_call_long(MTH_FLOG), F32_0_0);

    /* FEXP: exp(0.0) = 1.0  (e^0 = 1, exact) */
    ria_set_axsreg(F32_0_0);
    check_long("FEXP   exp(0.0)=1.0",
               (unsigned long)ria_call_long(MTH_FEXP), F32_1_0);

    /* FTOI: (int)3.0 = 3 */
    ria_set_axsreg(F32_3_0);
    check_long("FTOI   (int)3.0=3",
               (unsigned long)ria_call_long(MTH_FTOI), 3UL);

    /* FTOI: (int)-2.0 = -2  (negative float to int) */
    ria_set_axsreg(0xC0000000UL); /* -2.0f */
    check_long("FTOI   (int)-2.0=-2",
               (unsigned long)ria_call_long(MTH_FTOI), (unsigned long)-2L);

    /* ITOF: (float)5 = 5.0 = 0x40A00000 */
    ria_set_axsreg(5UL);
    check_long("ITOF   (float)5=5.0",
               (unsigned long)ria_call_long(MTH_ITOF), F32_5_0);

    /* ITOF: (float)0 = 0.0 */
    ria_set_axsreg(0UL);
    check_long("ITOF   (float)0=0.0",
               (unsigned long)ria_call_long(MTH_ITOF), F32_0_0);

    /* ==================== DOUBLE64 ==================== */

    /* IEEE 754 double bit patterns used below:
     *   1.0d = 0x3FF0000000000000  lo=0x00000000  hi=0x3FF00000
     *   1.5d = 0x3FF8000000000000  lo=0x00000000  hi=0x3FF80000
     *   2.0d = 0x4000000000000000  lo=0x00000000  hi=0x40000000
     *   3.0d = 0x4008000000000000  lo=0x00000000  hi=0x40080000
     *
     * After each double op, read result with two ria_pop_long() calls:
     *   first  = lo32 (bytes 0-3 of double, the low IEEE 754 bits)
     *   second = hi32 (bytes 4-7, contains sign/exponent/high mantissa)
     *
     * Push order: push_double(lo32, hi32) pushes hi32 first so that
     * lo32 ends up at the top of xstack as required by the firmware.
     */

    puts("\n-- Double64 --");

    /* DADD: 1.0 + 1.0 = 2.0 */
    push_double(0x00000000UL, 0x3FF00000UL); /* a = 1.0 */
    push_double(0x00000000UL, 0x3FF00000UL); /* b = 1.0 */
    ria_call_int(MTH_DADD);
    lo = ria_pop_long();
    hi = ria_pop_long();
    check_long("DADD   1.0+1.0=2.0 lo", lo, 0x00000000UL);
    check_long("DADD   1.0+1.0=2.0 hi", hi, 0x40000000UL);

    /* DMUL: 2.0 * 1.5 = 3.0 */
    push_double(0x00000000UL, 0x3FF80000UL); /* a = 1.5 */
    push_double(0x00000000UL, 0x40000000UL); /* b = 2.0 */
    ria_call_int(MTH_DMUL);
    lo = ria_pop_long();
    hi = ria_pop_long();
    check_long("DMUL   2.0*1.5=3.0 lo", lo, 0x00000000UL);
    check_long("DMUL   2.0*1.5=3.0 hi", hi, 0x40080000UL);

    /* DDIV: 3.0 / 1.5 = 2.0 */
    push_double(0x00000000UL, 0x40080000UL); /* a = 3.0 */
    push_double(0x00000000UL, 0x3FF80000UL); /* b = 1.5 */
    ria_call_int(MTH_DDIV);
    lo = ria_pop_long();
    hi = ria_pop_long();
    check_long("DDIV   3.0/1.5=2.0 lo", lo, 0x00000000UL);
    check_long("DDIV   3.0/1.5=2.0 hi", hi, 0x40000000UL);

    // sine_table();
    benchmark_sine();

    // cosine_table();
    benchmark_cosine();

    /* ==================== SUMMARY ==================== */

    puts("");
    printf("Results: %u passed, %u failed\n", passed, failed);
}
