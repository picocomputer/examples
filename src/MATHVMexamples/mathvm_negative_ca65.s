; Minimal ca65 / 6502 asm negative-test example for Picocomputer MATHVM.
;
; Executes five invalid v1 frames through OS $80:
;   0: bad magic
;   1: bad header
;   2: unsupported flag
;   3: stack underflow
;   4: bad local
;
; Results are stored in:
;   statuses[0..4]
;   out_words[0..4]
;
; Expected status bytes:
;   $01, $03, $0B, $07, $08

        .export _main

RIA_XSTACK      = $FFEC
RIA_OP          = $FFEF
RIA_OP_MATHVM   = $80
RIA_WAIT        = $FFF1

        .segment "BSS"
statuses:       .res 5
out_words:      .res 5

        .segment "CODE"

.macro RUN_CASE index, frame_label, frame_len
        .local push, drain, done

        ldx #frame_len - 1
push:
        lda frame_label, x
        sta RIA_XSTACK
        dex
        bpl push

        lda #RIA_OP_MATHVM
        sta RIA_OP
        jsr RIA_WAIT

        sta statuses + index
        stx out_words + index

        txa
        asl a
        asl a
        tax
        beq done

drain:
        lda RIA_XSTACK
        dex
        bne drain

done:
.endmacro

_main:
        RUN_CASE 0, bad_magic_frame, bad_magic_frame_len
        RUN_CASE 1, bad_header_frame, bad_header_frame_len
        RUN_CASE 2, unsupported_flag_frame, unsupported_flag_frame_len
        RUN_CASE 3, stack_underflow_frame, stack_underflow_frame_len
        RUN_CASE 4, bad_local_frame, bad_local_frame_len
        rts

        .segment "RODATA"

bad_magic_frame:
        .byte $00,$01,$00,$10,$02,$00,$00,$01,$FF,$FF,$FF,$FF,$01,$00,$00,$00
        .byte $02,$00
bad_magic_frame_len = * - bad_magic_frame

bad_header_frame:
        .byte $4D,$01,$00,$0F,$02,$00,$00,$01,$FF,$FF,$FF,$FF,$01,$00,$00,$00
        .byte $02,$00
bad_header_frame_len = * - bad_header_frame

unsupported_flag_frame:
        .byte $4D,$01,$10,$10,$02,$00,$00,$01,$FF,$FF,$FF,$FF,$01,$00,$00,$00
        .byte $02,$00
unsupported_flag_frame_len = * - unsupported_flag_frame

stack_underflow_frame:
        .byte $4D,$01,$00,$10,$01,$00,$00,$01,$FF,$FF,$FF,$FF,$01,$00,$00,$00
        .byte $19
stack_underflow_frame_len = * - stack_underflow_frame

bad_local_frame:
        .byte $4D,$01,$00,$10,$02,$01,$01,$02,$FF,$FF,$FF,$FF,$01,$00,$00,$00
        .byte $00,$00,$F6,$42
        .byte $12,$01
bad_local_frame_len = * - bad_local_frame
