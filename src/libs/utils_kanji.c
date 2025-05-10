/*
	Based in MC. MSX Commander source code.
	Thanks to Retro Canada (retrocanada76@gmail.com)
	...and to KdL for giving me the clue

	More info:
	https://www.msxcomputermagazine.nl/mccw/91/KANJI-stuff/en.html
*/
#include <stdint.h>
#include <stdbool.h>
#include "msx_const.h"


bool detectKanjiDriver() __naked __z88dk_fastcall
{
	__asm
		ld   l, #0
		call .check_extbios
		ret  z

		; KANJI-driver check - BiFi'96/'99
		; Out: Z-flag set when no driver
		;
		ld   a,#255
		ld   de, #0x1100	; D = ID-code, E = Command
		call EXTBIO
		inc  a
		ret  z
		inc  l
		ret

		.check_extbios:		; Z=No EXTBIOS | NZ=Exists EXTBIOS
			ld a, (HOKVLD)
			and #1
			ret
	__endasm;
}

char getKanjiMode() __naked __sdcccall(1)
{
	__asm
		call .check_extbios
		ret  z

		; Read KANJI-mode - BiFi'96/'99
		; Out: A=Current KANJI-mode
		;         =0   ANK
		;         =1   KANJI0
		;         =2   KANJI1
		;         =3   KANJI2
		;         =4   KANJI3
		;
		ld   de, #0x1100	; D = ID-code, E = Command
		jp   EXTBIO
	__endasm;
}

void setKanjiMode(uint8_t mode) __naked __z88dk_fastcall
{
	mode;
	__asm
		call .check_extbios
		ret  z

		; Set KANJI-mode - BiFi'96/'99
		; In:  A = KANJI-mode
		;
		ld   a, l
		ld   de, #0x1101	; D = ID-code, E = Command
		jp   EXTBIO
	__endasm;
}
