

;******************************************************************************
; DEBUG_BREAKPOINT
;	debug set_watchpoint read_io 0x18
;******************************************************************************
;	.macro DEBUG_BREAKPOINT
DEBUG_BREAKPOINT::
		push af
		out (0x18),a
		pop af
;	.endm
	ret



;******************************************************************************
; DEBUG_BORDERCOLOR
;******************************************************************************
;	.macro DEBUG_BORDERCOLOR value
DEBUG_BORDERCOLOR::
		push af
;		ld  a,#value
		out (0x99),a
		ld  a,#0x87
		out (0x99),a
		pop  af
;	.endm
	ret


;******************************************************************************
; DEBUG_PRINT
;******************************************************************************
PRINT_LINEFEED		= #0b00000000
PRINT_NOLINEFEED	= #0b01000000
PRINT_MULTIBYTE		= #0b00100000
PRINT_SINGLEBYTE	= #0b00010000
PRINT_HEXADECIMAL	= #0b00000000
PRINT_BINARY		= #0b00000001
PRINT_DECIMAL		= #0b00000010
PRINT_ASCII			= #0b00000011
PRINT_DEFAULTNUM	= PRINT_LINEFEED|PRINT_SINGLEBYTE|PRINT_HEXADECIMAL|PRINT_BINARY|PRINT_DECIMAL|PRINT_ASCII

	; A reg to print
;	.macro DEBUG_PRINT_A
DEBUG_PRINT_A::
		push af
		ld  a,#(PRINT_HEXADECIMAL|PRINT_MULTIBYTE)
		out (0x2e),a
		pop af
		out (0x2f),a
;	.endm
	ret

	; HL reg to print
;	.macro DEBUG_PRINT_HL
DEBUG_PRINT_HL::
		push af
		ld  a,#(PRINT_HEXADECIMAL|PRINT_MULTIBYTE)
		out (0x2e),a
		ld  a,h
		out (0x2f),a
		ld  a,l
		out (0x2f),a
		pop af
;	.endm
	ret

	; DE reg to print
;	.macro DEBUG_PRINT_DE
DEBUG_PRINT_DE::
		push af
		ld  a,#(PRINT_HEXADECIMAL|PRINT_MULTIBYTE)
		out (0x2e),a
		ld  a,d
		out (0x2f),a
		ld  a,e
		out (0x2f),a
		pop af
;	.endm
	ret

	; BC reg to print
;	.macro DEBUG_PRINT_BC
DEBUG_PRINT_BC::
		push af
		ld  a,#(PRINT_HEXADECIMAL|PRINT_MULTIBYTE)
		out (0x2e),a
		ld  a,b
		out (0x2f),a
		ld  a,c
		out (0x2f),a
		pop af
;	.endm
	ret

;	.macro DEBUG_PRINT_NUM value
;		push af
;		ld  a,#PRINT_DEFAULTNUM
;		out (0x2e),a
;		ld  a,#value
;		out (0x2f),a
;		pop af
;	.endm

	; A ascii char
;	.macro DEBUG_PRINT_ASCII
DEBUG_PRINT_ASCII::
		push af
		ld  a,#(PRINT_LINEFEED|PRINT_MULTIBYTE|PRINT_ASCII)
		out (0x2e),a
		pop af
		out (0x2f),a
;	.endm
	ret

	; HL ascii string
	; B  string size
;	.macro DEBUG_PRINT_ASCII_TEXT
DEBUG_PRINT_ASCII_TEXT::
		push bc
		push af
		ld  a,#(PRINT_LINEFEED|PRINT_MULTIBYTE|PRINT_ASCII)
		out (0x2e),a
		pop af
		ld  c,#0x2f
		otir
		pop bc
;	.endm
	ret
