; https://map.grauw.nl/sources/callbios.php
;
; CALSUB
;
; In: IX = address of routine in MSX2 SUBROM
;     AF, HL, DE, BC = parameters for the routine
;
; Out: AF, HL, DE, BC = depending on the routine
;
; Changes: IX, IY, AF', BC', DE', HL'
;
; Call MSX2 subrom from MSXDOS. Should work with all versions of MSXDOS.
;
; Notice: NMI hook will be changed. This should pose no problem as NMI is
; not supported on the MSX at all.
;
CALSLT	.equ	#0x001C
NMI	.equ	#0x0066
EXTROM	.equ	#0x015F
EXPTBL	.equ	#0xFCC1
H_NMI	.equ	#0xFDD6
;
CALSUB::
	EXX
	EX     AF,AF'       		; store all registers
	LD     HL,#EXTROM
	PUSH   HL
	LD     HL,#0xC300
	PUSH   HL         			; push NOP ; JP EXTROM
	PUSH   IX
	LD     HL,#0x21DD
	PUSH   HL					; push LD IX,<entry>
	LD     HL,#0x3333
	PUSH   HL					; push INC SP; INC SP
	LD     HL,#0
	ADD    HL,SP				; HL = offset of routine
	LD     A,#0xC3
	LD     (#H_NMI),A
	LD     (#H_NMI+1),HL		; JP <routine> in NMI hook
	EX     AF,AF'
	EXX							; restore all registers
	LD     IX,#NMI
	LD     IY,(#EXPTBL-1)
	CALL   CALSLT				; call NMI-hook via NMI entry in ROMBIOS
								; NMI-hook will call SUBROM
	EXX
	EX     AF,AF'				; store all returned registers
	LD     HL,#10
	ADD    HL,SP
	LD     SP,HL				; remove routine from stack
	EX     AF,AF'
	EXX							; restore all returned registers
	RET
