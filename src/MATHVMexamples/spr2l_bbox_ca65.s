; Minimal ca65 / 6502 asm example for Picocomputer MATHVM.
;
; Executes:
;   SPR2L 0,6,0x02
;   RET   4
;
; Expected float32 result words:
;   42B80000, 42380000, 42D80000, 42580000

        .export _main

RIA_XSTACK      = $FFEC
RIA_OP          = $FFEF
RIA_OP_MATHVM   = $80
RIA_WAIT        = $FFF1

        .segment "BSS"
status:         .res 1
out_words:      .res 1
result_bytes:   .res 16

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
        .byte $4D,$01,$00,$10,$06,$0A,$04,$08,$FF,$FF,$FF,$FF,$01,$00,$00,$00

        ; locals[0..9]
        ; affine 2x3: identity + translation (100,50)
        .byte $00,$00,$80,$3F
        .byte $00,$00,$00,$00
        .byte $00,$00,$C8,$42
        .byte $00,$00,$00,$00
        .byte $00,$00,$80,$3F
        .byte $00,$00,$48,$42

        ; sprite descriptor: w=16, h=8, ax=0.5, ay=0.5
        .byte $00,$00,$80,$41
        .byte $00,$00,$00,$41
        .byte $00,$00,$00,$3F
        .byte $00,$00,$00,$3F

        ; program: SPR2L 0,6,0x02 ; RET 4
        .byte $48,$00,$06,$02,$02,$04

frame_len = * - frame
