/*
 * Standalone copy for examples.
 * Keep in sync with ../mathvm_client.h as needed by on-target callers.
 */

#ifndef _MATHVM_EXAMPLES_MATHVM_CLIENT_H_
#define _MATHVM_EXAMPLES_MATHVM_CLIENT_H_

#include "mathvm.h"

typedef struct
{
    uint8_t status;
    uint8_t out_words;
} mx_client_result_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} mx_vec3i_t;

typedef struct
{
    int16_t x;
    int16_t y;
} mx_point2i_t;

typedef struct
{
    uint8_t flags;
    uint16_t xram_in;
    uint16_t xram_out;
    uint16_t count;
    const mx_word_t *locals;
    uint8_t local_words;
    const uint8_t *program;
    uint8_t prog_len;
    uint8_t out_words;
    uint8_t stack_words;
} mx_client_batch_desc_t;

mx_client_result_t mx_client_call_frame(const uint8_t *frame,
                                        uint16_t frame_len,
                                        mx_word_t *out,
                                        uint8_t out_cap_words);

mx_client_result_t mx_client_call_batch(const mx_client_batch_desc_t *desc,
                                        mx_word_t *out,
                                        uint8_t out_cap_words);

mx_client_result_t mx_client_m3v3l(const mx_word_t mat3[9],
                                   const mx_word_t vec3[3],
                                   mx_word_t out[3]);

mx_client_result_t mx_client_spr2l_bbox(const mx_word_t affine2x3[6],
                                        const mx_word_t sprite[4],
                                        mx_word_t out[4]);

mx_client_result_t mx_client_spr2l_corners(const mx_word_t affine2x3[6],
                                           const mx_word_t sprite[4],
                                           mx_word_t out[8]);

void mx_client_xram_write_vec3i_array(uint16_t xram_addr,
                                      const mx_vec3i_t *vecs,
                                      uint16_t count);

void mx_client_xram_read_point2i_array(uint16_t xram_addr,
                                       mx_point2i_t *points,
                                       uint16_t count);

mx_client_result_t mx_client_m3v3p2x(const mx_word_t mat3[9],
                                     const mx_word_t camera[3],
                                     uint16_t xram_in,
                                     uint16_t xram_out,
                                     uint16_t count);

mx_client_result_t mx_client_m3v3p2x_q8_8(const int16_t mat3_q8_8[9],
                                          int16_t persp_d,
                                          int16_t screen_cx,
                                          int16_t screen_cy,
                                          uint16_t xram_in,
                                          uint16_t xram_out,
                                          uint16_t count);

mx_client_result_t mx_client_m3v3p2x_yrot30(int angle_deg,
                                            int16_t persp_d,
                                            int16_t screen_cx,
                                            int16_t screen_cy,
                                            uint16_t xram_in,
                                            uint16_t xram_out,
                                            uint16_t count);

mx_client_result_t mx_client_project_vec3i_batch_yrot30(int angle_deg,
                                                        int16_t persp_d,
                                                        int16_t screen_cx,
                                                        int16_t screen_cy,
                                                        uint16_t xram_base,
                                                        const mx_vec3i_t *vecs,
                                                        mx_point2i_t *points,
                                                        uint16_t count);

#endif /* _MATHVM_EXAMPLES_MATHVM_CLIENT_H_ */
