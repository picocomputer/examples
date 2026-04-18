/*
 * Minimal cc65 example for Picocomputer 6502 MATHVM.
 *
 * Executes:
 *   M3V3L 0,9
 *   RET   3
 *
 * Expected output words as float32:
 *   140.0f, 320.0f, 500.0f
 */

#include "mathvm_client.h"
#include <stdio.h>

int main(void)
{
    static const uint32_t mat3_bits[9] = {
        0x3F800000UL, 0x40000000UL, 0x40400000UL,
        0x40800000UL, 0x40A00000UL, 0x40C00000UL,
        0x40E00000UL, 0x41000000UL, 0x41100000UL,
    };
    static const uint32_t vec3_bits[3] = {
        0x41200000UL, 0x41A00000UL, 0x41F00000UL,
    };
    mx_word_t mat3[9];
    mx_word_t vec3[3];
    mx_client_result_t call;
    mx_word_t result[3];
    uint8_t i;

    for (i = 0; i < 9; ++i)
        mat3[i].u32 = mat3_bits[i];
    for (i = 0; i < 3; ++i)
        vec3[i].u32 = vec3_bits[i];

    call = mx_client_m3v3l(mat3, vec3, result);

    printf("MATHVM status=%u words=%u\n", call.status, call.out_words);

    if (call.status == 0 && call.out_words == 3)
    {
        printf("result[0]=%08lx\n", (unsigned long)result[0].u32);
        printf("result[1]=%08lx\n", (unsigned long)result[1].u32);
        printf("result[2]=%08lx\n", (unsigned long)result[2].u32);
        puts("expected: 430C0000 43A00000 43FA0000");
    }

    return 0;
}
