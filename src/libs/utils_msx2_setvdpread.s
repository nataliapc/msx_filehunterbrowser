;===============================================================================
; Set VDP address counter to read/write from address A+DE (17-bit)
; SSRRRRRRRRCCCCCCC     SCREEN 5/6:  S=Screen  R=Row  C=Column
; SRRRRRRRRCCCCCCCC     SCREEN 7/8:  S=Screen  R=Row  C=Column
; ADDDDDDDDEEEEEEEE     Registers
; In:   A+DE    VRAM address
; Used: A,C,D
;

;void setVDP_Read(uint32_t vram) __sdcccall(1);
_setVDP_Read::
	ld      a,l			; HL:DE = Param vram
setVDP_Read::
	ld      c,#0x00
	jr      .jmp1

;void setVDP_Write(uint32_t vram) __sdcccall(1);
_setVDP_Write::
	ld      a,l			; HL:DE = Param vram
setVDP_Write::
	ld      c,#0x40
.jmp1:
	rlc     d
	rla
	rlc     d
	rla
	srl     d
	srl     d
	di
	out     (0x99), a
	ld      a,#(14+128)
	out     (0x99), a
	ld      a,e
	nop
	out     (0x99),a
	ld      a,d
	or      c
	ei
	out     (0x99),a
	ret
