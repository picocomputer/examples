; Minimal ca65 / 6502 asm example for Picocomputer MATHVM.
;
; Executes:
;   M3V3L 0,9
;   RET   3
;
; Expected float32 result words:
;   430C0000, 43A00000, 43FA0000

        .export _main

RIA_XSTACK      = $FFEC
RIA_OP          = $FFEF
RIA_OP_MATHVM   = $80
RIA_WAIT        = $FFF1

        .segment "BSS"
status:         .res 1
out_words:      .res 1
result_bytes:   .res 12

        .segment "CODE"

_main:
        ldx #frame_len - 1
@push:
        lda frame, x
        sta RIA_XSTACK
        dex
        bpl @push

        lda #RIA_OP_MATHVM
        sta RIA_OP
        jsr RIA_WAIT

        sta status
        stx out_words

        txa
        asl a
        asl a
        tax
        beq @done

        ldy #$00
@read:
        lda RIA_XSTACK
        sta result_bytes, y
        iny
        dex
        bne @read

@done:
        rts

        .segment "RODATA"

frame:
        ; header
        .byte $4D,$01,$00,$10,$05,$0C,$03,$08,$FF,$FF,$FF,$FF,$01,$00,$00,$00

        ; locals[0..11]
        ; mat3 row-major: 1.0 .. 9.0
        .byte $00,$00,$80,$3F
        .byte $00,$00,$00,$40
        .byte $00,$00,$40,$40
        .byte $00,$00,$80,$40
        .byte $00,$00,$A0,$40
        .byte $00,$00,$C0,$40
        .byte $00,$00,$E0,$40
        .byte $00,$00,$00,$41
        .byte $00,$00,$10,$41

        ; vec3: 10.0, 20.0, 30.0
        .byte $00,$00,$20,$41
        .byte $00,$00,$A0,$41
        .byte $00,$00,$F0,$41

        ; program: M3V3L 0,9 ; RET 3
        .byte $3E,$00,$09,$02,$03

frame_len = * - frame
