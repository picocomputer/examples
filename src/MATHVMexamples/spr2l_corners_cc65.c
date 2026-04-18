/*
 * Minimal cc65 example for Picocomputer 6502 MATHVM.
 *
 * Executes:
 *   SPR2L 0,6,0x01
 *   RET   8
 *
 * Expected output words as float32:
 *   92.0f, 46.0f, 108.0f, 46.0f,
 *   108.0f, 54.0f, 92.0f, 54.0f
 */

#include "mathvm_client.h"
#include <stdio.h>

int main(void)
{
    static const uint32_t affine2x3_bits[6] = {
        0x3F800000UL, 0x00000000UL, 0x42C80000UL,
        0x00000000UL, 0x3F800000UL, 0x42480000UL,
    };
    static const uint32_t sprite_bits[4] = {
        0x41800000UL, 0x41000000UL,
        0x3F000000UL, 0x3F000000UL,
    };
    mx_word_t affine2x3[6];
    mx_word_t sprite[4];
    mx_client_result_t call;
    mx_word_t result[8];
    uint8_t i;

    for (i = 0; i < 6; ++i)
        affine2x3[i].u32 = affine2x3_bits[i];
    for (i = 0; i < 4; ++i)
        sprite[i].u32 = sprite_bits[i];

    call = mx_client_spr2l_corners(affine2x3, sprite, result);

    printf("MATHVM status=%u words=%u\n", call.status, call.out_words);

    if (call.status == 0 && call.out_words == 8)
    {
        for (i = 0; i < 8; ++i)
            printf("corner[%u]=%08lx\n", i, (unsigned long)result[i].u32);
        puts("expected:");
        puts("  42B80000 42380000 42D80000 42380000");
        puts("  42D80000 42580000 42B80000 42580000");
    }

    return 0;
}
