/*
 * Standalone copy for examples.
 * Keep in sync with ../mathvm.h as needed by on-target callers.
 */

#ifndef _MATHVM_EXAMPLES_MATHVM_H_
#define _MATHVM_EXAMPLES_MATHVM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RIA_OP_MATHVM 0x80

#define MX_WORD_BYTES 4u
#define MX_HEADER_BYTES 16u
#define MX_MAX_LOCALS 48u
#define MX_MAX_STACK 24u
#define MX_MAX_PROG 160u
#define MX_MAX_OUT 8u
#define MX_MAX_STEPS 8192u
#define MX_MAX_FRAME (MX_HEADER_BYTES + (MX_MAX_LOCALS * MX_WORD_BYTES) + MX_MAX_PROG)

typedef union
{
    uint32_t u32;
    int32_t i32;
    float f32;
} mx_word_t;

typedef struct
{
    uint8_t magic;
    uint8_t version;
    uint8_t flags;
    uint8_t hdr_size;
    uint8_t prog_len;
    uint8_t local_words;
    uint8_t out_words;
    uint8_t stack_words;
    uint16_t xram_in;
    uint16_t xram_out;
    uint16_t count;
    uint16_t reserved;
} mx_header_t;

enum
{
    MX_FLAG_USE_XRAM_IN = 0x01,
    MX_FLAG_USE_XRAM_OUT = 0x02,
    MX_FLAG_RETURN_I16 = 0x04,
    MX_FLAG_SATURATE = 0x08,
    MX_FLAG_DEBUG = 0x10,
    MX_FLAG_RESERVED5 = 0x20,
    MX_FLAG_RESERVED6 = 0x40,
    MX_FLAG_RESERVED7 = 0x80,
};

typedef enum
{
    MX_OK = 0x00,
    MX_ERR_MAGIC = 0x01,
    MX_ERR_VERSION = 0x02,
    MX_ERR_HEADER = 0x03,
    MX_ERR_PROGRAM = 0x04,
    MX_ERR_BAD_OPCODE = 0x05,
    MX_ERR_STACK_OVF = 0x06,
    MX_ERR_STACK_UDF = 0x07,
    MX_ERR_BAD_LOCAL = 0x08,
    MX_ERR_BAD_XRAM = 0x09,
    MX_ERR_NUMERIC = 0x0A,
    MX_ERR_UNSUPPORTED = 0x0B,
} mx_status_t;

typedef enum
{
    MX_NOP = 0x00,
    MX_HALT = 0x01,
    MX_RET = 0x02,
    MX_MUL8U = 0x03,
    MX_MUL16U = 0x04,
    MX_MUL16S = 0x05,
    MX_DIV16U = 0x06,
    MX_SQRT32U = 0x07,
    MX_PUSHF = 0x10,
    MX_PUSHI = 0x11,
    MX_LDS = 0x12,
    MX_STS = 0x13,
    MX_LDV2 = 0x14,
    MX_STV2 = 0x15,
    MX_LDV3 = 0x16,
    MX_STV3 = 0x17,
    MX_DUP = 0x18,
    MX_DROP = 0x19,
    MX_SWAP = 0x1A,
    MX_OVER = 0x1B,
    MX_LDD = 0x1C,
    MX_STD = 0x1D,
    MX_FADD = 0x20,
    MX_FSUB = 0x21,
    MX_FMUL = 0x22,
    MX_FDIV = 0x23,
    MX_FMADD = 0x24,
    MX_FNEG = 0x25,
    MX_FABS = 0x26,
    MX_FSQRT = 0x27,
    MX_FSIN = 0x28,
    MX_FCOS = 0x29,
    MX_FMIN = 0x2A,
    MX_FMAX = 0x2B,
    MX_FLOOR = 0x2C,
    MX_FCEIL = 0x2D,
    MX_FROUND = 0x2E,
    MX_FTRUNC = 0x2F,
    MX_V2ADD = 0x30,
    MX_V2SUB = 0x31,
    MX_V2DOT = 0x32,
    MX_V2SCALE = 0x33,
    MX_A2P2L = 0x34,
    MX_V3ADD = 0x38,
    MX_V3SUB = 0x39,
    MX_V3DOT = 0x3A,
    MX_V3SCALE = 0x3B,
    MX_CROSS3 = 0x3C,
    MX_NORM3 = 0x3D,
    MX_M3V3L = 0x3E,
    MX_M3M3L = 0x3F,
    MX_FATAN2 = 0x40,
    MX_FPOW = 0x41,
    MX_FLOG = 0x42,
    MX_FEXP = 0x43,
    MX_FTOI = 0x44,
    MX_ITOF = 0x45,
    MX_DADD = 0x46,
    MX_DMUL = 0x47,
    MX_SPR2L = 0x48,
    MX_BBOX2 = 0x49,
    MX_ROUND2I = 0x4A,
    MX_DDIV = 0x4B,
    MX_M3V3P2X = 0x4C,
    MX_CMPZ = 0x60,
    MX_FCMPLT = 0x61,
    MX_FCMPGT = 0x62,
    MX_JMP = 0x63,
    MX_JZ = 0x64,
    MX_JNZ = 0x65,
    MX_SELECT = 0x66,
} mx_opcode_t;

#endif /* _MATHVM_EXAMPLES_MATHVM_H_ */
