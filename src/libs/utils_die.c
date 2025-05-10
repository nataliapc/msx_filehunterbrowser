#include "utils.h"
#include "dos.h"
#include "conio.h"


void die(const char *s)
{
	// Set SCREEN 0 (text mode)
/*
	__asm
		xor  a
		ld   iy,(#0xfcc1-1)		; EXPTBL
		ld   ix,#0x5f
		call 0x1c				; CALSTL
	__endasm;
*/
	cputs(s);
	exit();
}